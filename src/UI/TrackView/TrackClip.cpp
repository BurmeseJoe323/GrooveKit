// JUNIE
#include "TrackClip.h"
#include "TrackEditView.h"
#include "TrackComponent.h"
#include "TrackListComponent.h"

TrackClip::TrackClip (te::MidiClip* c, float pixelsPerBeat)
    : clip (c),
      pixelsPerBeat (pixelsPerBeat),
      resizeConstrainer (*this),
      edgeResizer (this, &resizeConstrainer, juce::ResizableEdgeComponent::Edge::rightEdge)
{
    jassert (clip != nullptr);

    // Attach to the clip's ValueTree state to listen for length changes
    if (clip != nullptr)
    {
        clipState = clip->state;
        if (clipState.isValid())
            clipState.addListener (this);
    }

    // Set minimum width constraint for resizing
    resizeConstrainer.setMinimumWidth (20);

    addAndMakeVisible (edgeResizer);
    updateSizeFromClip();
}

TrackClip::~TrackClip()
{
    if (clipState.isValid())
        clipState.removeListener (this);
}

void TrackClip::updateSizeFromClip()
{
    if (!clip)
        return;

    // Use the same calculation as TrackComponent::resized() for consistency
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (tl)
    {
        const double pixelsPerSecond = tl->getPixelsPerSecond();
        const double lengthSecs = clip->getPosition().getLength().inSeconds();
        const int w = static_cast<int> (juce::roundToIntAccurate (lengthSecs * pixelsPerSecond));

        // Keep X/Y/Height unchanged; only update width
        auto b = getBounds();
        setBounds (b.withWidth (juce::jmax (20, w)));
        repaint();
    }
    else
    {
        // Fallback to old method if parent not found
        const double lengthBeats = clip->getLengthInBeats().inBeats();
        const int w = juce::roundToInt (lengthBeats * pixelsPerBeat);
        auto b = getBounds();
        setBounds (b.withWidth (juce::jmax (1, w)));
        repaint();
    }
}

void TrackClip::setPixelsPerBeat (float ppb)
{
    pixelsPerBeat = ppb;
    updateSizeFromClip();
}

void TrackClip::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    constexpr float radius = 8.0f;

    // Fill - Apply drag alpha for transparency during drag (Written by Claude Code)
    g.setColour (clipColor.withAlpha (dragAlpha));
    g.fillRoundedRectangle (r, radius);

    // Border
    g.setColour (juce::Colours::white.withAlpha (0.35f * dragAlpha));
    g.drawRoundedRectangle (r.reduced (0.5f), radius, 1.0f);

    // Label
    g.setColour (juce::Colours::white.withAlpha (dragAlpha));
    g.drawText ("MIDI Clip", getLocalBounds(), juce::Justification::centred);
}

void TrackClip::resized()
{
    constexpr int handleWidth = 8;
    edgeResizer.setBounds (getWidth() - handleWidth, 0, handleWidth, getHeight());
}

void TrackClip::onResizeEnd()
{
    if (!clip)
        return;

    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (!tl)
        return;

    const double pixelsPerSecond = tl->getPixelsPerSecond();
    if (pixelsPerSecond <= 0.0)
        return;

    // Calculate the new length based on the current width
    const double newLengthSecs = static_cast<double> (getWidth()) / pixelsPerSecond;

    // Update the model - preserveSync=false since we're manually resizing
    clip->setLength (te::TimeDuration::fromSeconds (newLengthSecs), false);

    // Update only the width from the model, using the same calculation as TrackComponent::resized()
    // This preserves the X position and avoids visual "jump"
    const double actualLength = clip->getPosition().getLength().inSeconds();
    const int correctWidth = static_cast<int> (juce::roundToIntAccurate (actualLength * pixelsPerSecond));

    auto b = getBounds();
    setBounds (b.withWidth (juce::jmax (correctWidth, 20)));
}

// Drag helper methods - Written by Claude Code
te::TimePosition TrackClip::mouseToTime (const juce::MouseEvent& e)
{
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (!tl)
        return te::TimePosition::fromSeconds (0.0);

    const double pixelsPerSecond = tl->getPixelsPerSecond();
    const double viewStartSec = tl->getViewStart().inSeconds();

    // Convert event to TrackListComponent coordinates
    auto eventInTrackList = e.getEventRelativeTo (tl);
    const int globalX = eventInTrackList.x;

    // Account for header width - timeline starts after the header (Written by Claude Code)
    constexpr int headerWidth = 140;
    const int timelineX = globalX - headerWidth;

    const double timeSec = viewStartSec + (static_cast<double> (timelineX) / pixelsPerSecond);
    return te::TimePosition::fromSeconds (timeSec);
}

int TrackClip::mouseToTrackIndex (const juce::MouseEvent& e)
{
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (!tl)
        return -1;

    // Convert event to TrackListComponent coordinates
    auto eventInTrackList = e.getEventRelativeTo (tl);

    // Account for timeline height at top
    constexpr int timelineHeight = 24;
    constexpr int trackHeight = 125;

    const int y = eventInTrackList.y - timelineHeight;
    if (y < 0)
        return -1;

    return y / trackHeight;
}

te::TimePosition TrackClip::quantizeToGrid (te::TimePosition time, double gridSize)
{
    const double seconds = time.inSeconds();
    const double quantized = std::round (seconds / gridSize) * gridSize;
    return te::TimePosition::fromSeconds (juce::jmax (0.0, quantized));
}

void TrackClip::mouseDown (const juce::MouseEvent& e)
{
    // Written by Claude Code
    // Check if clicking on resize handle vs clip body
    if (e.originalComponent == &edgeResizer)
    {
        // Let ResizableEdgeComponent handle resizing
        return;
    }

    // Don't start drag on right-click (used for context menu)
    if (e.mods.isPopupMenu())
        return;

    // Initiate drag
    isDragging = true;
    dragStartMousePos = e.getScreenPosition();
    originalStartTime = clip->getPosition().getStart();

    // Find our track index
    if (auto* trackComp = findParentComponentOfClass<TrackComponent>())
        originalTrackIndex = trackComp->getTrackIndex();

    setMouseCursor (juce::MouseCursor::DraggingHandCursor);
}

void TrackClip::mouseDrag (const juce::MouseEvent& e)
{
    // Written by Claude Code
    if (!isDragging || !clip)
        return;

    // Calculate target position with quantization
    te::TimePosition targetTime = mouseToTime (e);
    te::TimePosition quantizedTime = quantizeToGrid (targetTime);

    // Calculate target track
    int targetTrackIndex = mouseToTrackIndex (e);

    // Get clip length for preview
    const auto clipLength = clip->getPosition().getLength();

    // Notify parent for validation and ghost preview
    if (onDragUpdate)
    {
        // Parent will validate and show ghost
        bool isValid = true; // Parent will determine this
        onDragUpdate (targetTrackIndex, quantizedTime, clipLength, isValid);
    }

    // Make clip semi-transparent during drag
    dragAlpha = 0.5f;
    repaint();
}

void TrackClip::mouseUp (const juce::MouseEvent& e)
{
    // Written by Claude Code
    if (isDragging)
    {
        isDragging = false;
        dragAlpha = 1.0f;
        setMouseCursor (juce::MouseCursor::NormalCursor);

        // Calculate final drop position
        te::TimePosition targetTime = mouseToTime (e);
        te::TimePosition quantizedTime = quantizeToGrid (targetTime);
        int targetTrackIndex = mouseToTrackIndex (e);

        // Notify parent to apply changes
        if (onDragComplete)
            onDragComplete (clip, targetTrackIndex, quantizedTime);

        repaint();
        return;
    }

    // Right click: delegate context menu handling to the parent TrackComponent
    if (e.mods.isPopupMenu())
    {
        if (onContextMenuRequested)
            onContextMenuRequested (clip);
    }
}

void TrackClip::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown() && onClicked)
        onClicked (clip);
}

void TrackClip::setColor (juce::Colour newColor)
{
    clipColor = newColor;
    repaint();
}

void TrackClip::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    juce::ignoreUnused (tree);

    if (property == te::IDs::length)
    {
        // Ensure UI updates happen on the message thread
        juce::Component::SafePointer<TrackClip> safeThis (this);
        juce::MessageManager::callAsync ([safeThis] {
            if (safeThis != nullptr)
                safeThis->updateSizeFromClip();
        });
    }
}