#include "TrackClip.h"
#include "TrackEditView.h"
#include "TrackComponent.h"
#include "TrackListComponent.h"
#include <limits> // For std::numeric_limits

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

    // Use the same calculation as TrackComponent::resized() for consistency (Written by Claude Code)
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (tl)
    {
        const double ppb = tl->getPixelsPerBeat();
        auto& tempoSeq = clip->edit.tempoSequence;

        // Convert time positions to beat positions for width calculation
        const auto posRange = clip->getPosition().time;
        const double clipStartBeats = tempoSeq.toBeats (posRange.getStart()).inBeats();
        const double clipEndBeats = tempoSeq.toBeats (posRange.getEnd()).inBeats();
        const double clipLenBeats = clipEndBeats - clipStartBeats;

        const int w = static_cast<int> (juce::roundToIntAccurate (clipLenBeats * ppb));

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
    // Written by Claude Code
    const auto r = getLocalBounds().toFloat();
    constexpr float radius = 8.0f;

    // Fill - Apply drag alpha for transparency during drag
    g.setColour (clipColor.withAlpha (dragAlpha));
    g.fillRoundedRectangle (r, radius);

    // Border - highlight if being edited in piano roll
    if (isBeingEdited)
    {
        // Bright cyan border for edited clip
        g.setColour (juce::Colours::cyan.brighter (0.3f));
        g.drawRoundedRectangle (r.reduced (0.5f), radius, 2.5f);

        // Subtle outer glow
        g.setColour (juce::Colours::cyan.withAlpha (0.35f));
        g.drawRoundedRectangle (r.expanded (1.0f), radius + 1.0f, 1.5f);
    }
    else
    {
        // Normal white border
        g.setColour (juce::Colours::white.withAlpha (0.35f * dragAlpha));
        g.drawRoundedRectangle (r.reduced (0.5f), radius, 1.0f);
    }
}

void TrackClip::resized()
{
    constexpr int handleWidth = 8;
    edgeResizer.setBounds (getWidth() - handleWidth, 0, handleWidth, getHeight());
}

void TrackClip::onResizeEnd()
{
    // Written by Claude Code (fixed seconds/beats conversion)
    if (!clip)
        return;

    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (!tl)
        return;

    const double ppb = tl->getPixelsPerBeat();
    if (ppb <= 0.0)
        return;

    auto& tempoSeq = clip->edit.tempoSequence;

    // Calculate the new length based on the current width (pixels → beats)
    const double newLengthBeats = static_cast<double> (getWidth()) / ppb;

    // Quantize length to 0.25 beat grid
    constexpr double gridSize = 0.25;
    double quantizedLengthBeats = std::round (newLengthBeats / gridSize) * gridSize;
    double finalLengthBeats = juce::jmax (gridSize, quantizedLengthBeats); // Minimum one grid unit

    // Check for overlap with other clips and constrain resize if needed
    auto* trackComp = findParentComponentOfClass<TrackComponent>();
    if (trackComp)
    {
        const auto clipStart = clip->getPosition().getStart();
        const double clipStartBeats = tempoSeq.toBeats (clipStart).inBeats();

        // Find the nearest clip that starts after this one
        double nearestClipStartBeats = std::numeric_limits<double>::max();

        // Get AppEngine through TrackListComponent to access clips
        for (int i = 0; i < trackComp->getNumChildComponents(); ++i)
        {
            if (auto* otherClipUI = dynamic_cast<TrackClip*> (trackComp->getChildComponent (i)))
            {
                if (otherClipUI == this)
                    continue; // Skip self

                auto* otherClip = otherClipUI->getMidiClip();
                if (!otherClip)
                    continue;

                const auto otherStart = otherClip->getPosition().getStart();
                const double otherStartBeats = tempoSeq.toBeats (otherStart).inBeats();

                // Check if this clip starts after our clip
                if (otherStartBeats > clipStartBeats)
                    nearestClipStartBeats = std::min (nearestClipStartBeats, otherStartBeats);
            }
        }

        // If we found a clip that would be overlapped, constrain the resize
        if (nearestClipStartBeats < std::numeric_limits<double>::max())
        {
            const double maxAllowedLengthBeats = nearestClipStartBeats - clipStartBeats;
            if (finalLengthBeats > maxAllowedLengthBeats)
            {
                // Constrain to just before the next clip, quantized
                const double constrainedLengthBeats = std::floor (maxAllowedLengthBeats / gridSize) * gridSize;
                finalLengthBeats = juce::jmax (gridSize, constrainedLengthBeats);
            }
        }
    }

    // Convert beats to TimeDuration using tempo sequence
    const auto clipStart = clip->getPosition().getStart();
    const double clipStartBeats = tempoSeq.toBeats (clipStart).inBeats();
    const double clipEndBeats = clipStartBeats + finalLengthBeats;

    const auto startTime = tempoSeq.toTime (t::BeatPosition::fromBeats (clipStartBeats));
    const auto endTime = tempoSeq.toTime (t::BeatPosition::fromBeats (clipEndBeats));
    const auto finalDuration = endTime - startTime;

    // Update the model - preserveSync=true prevents offset adjustment (Written by Claude Code)
    // This keeps the clip content at the same timeline position and removes notes from the END
    clip->setLength (finalDuration, true);

    // Update only the width from the model, using the same calculation as TrackComponent::resized()
    // This preserves the X position and avoids visual "jump"
    const auto posRange = clip->getPosition().time;
    const double actualStartBeats = tempoSeq.toBeats (posRange.getStart()).inBeats();
    const double actualEndBeats = tempoSeq.toBeats (posRange.getEnd()).inBeats();
    const double actualLengthBeats = actualEndBeats - actualStartBeats;
    const int correctWidth = static_cast<int> (juce::roundToIntAccurate (actualLengthBeats * ppb));

    auto b = getBounds();
    setBounds (b.withWidth (juce::jmax (correctWidth, 20)));

    // Trigger layout update in TrackListComponent to recalculate total width (Written by Claude Code)
    // This ensures the viewport expands when clips are resized past the visible edge
    if (tl)
        tl->resized();
}

void TrackClip::quantizeWidth (juce::Rectangle<int>& bounds)
{
    // Written by Claude Code - Live quantization during resize drag
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (!tl)
        return;

    const double ppb = tl->getPixelsPerBeat();
    if (ppb <= 0.0)
        return;

    // Calculate the new length in beats from the proposed width
    const double newLengthBeats = static_cast<double> (bounds.getWidth()) / ppb;

    // Quantize to 0.25 beat grid
    constexpr double gridSize = 0.25;
    double quantizedLengthBeats = std::round (newLengthBeats / gridSize) * gridSize;
    double finalLengthBeats = juce::jmax (gridSize, quantizedLengthBeats); // Minimum one grid unit

    // Check for overlap with adjacent clips and constrain if needed
    auto* trackComp = findParentComponentOfClass<TrackComponent>();
    if (trackComp && clip)
    {
        auto& tempoSeq = clip->edit.tempoSequence;
        const auto clipStart = clip->getPosition().getStart();
        const double clipStartBeats = tempoSeq.toBeats (clipStart).inBeats();

        // Find the nearest clip that starts after this one
        double nearestClipStartBeats = std::numeric_limits<double>::max();

        for (int i = 0; i < trackComp->getNumChildComponents(); ++i)
        {
            if (auto* otherClipUI = dynamic_cast<TrackClip*> (trackComp->getChildComponent (i)))
            {
                if (otherClipUI == this)
                    continue;

                auto* otherClip = otherClipUI->getMidiClip();
                if (!otherClip)
                    continue;

                const auto otherStart = otherClip->getPosition().getStart();
                const double otherStartBeats = tempoSeq.toBeats (otherStart).inBeats();

                if (otherStartBeats > clipStartBeats)
                    nearestClipStartBeats = std::min (nearestClipStartBeats, otherStartBeats);
            }
        }

        // Constrain to not overlap adjacent clip
        if (nearestClipStartBeats < std::numeric_limits<double>::max())
        {
            const double maxAllowedLengthBeats = nearestClipStartBeats - clipStartBeats;
            if (finalLengthBeats > maxAllowedLengthBeats)
            {
                const double constrainedLengthBeats = std::floor (maxAllowedLengthBeats / gridSize) * gridSize;
                finalLengthBeats = juce::jmax (gridSize, constrainedLengthBeats);
            }
        }
    }

    // Convert back to pixels and update the bounds width
    const int quantizedWidth = static_cast<int> (juce::roundToIntAccurate (finalLengthBeats * ppb));
    bounds.setWidth (juce::jmax (20, quantizedWidth)); // Minimum 20 pixels
}

t::TimePosition TrackClip::mouseToTime (const juce::MouseEvent& e)
{
    // Written by Claude Code (fixed seconds/beats conversion)
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (!tl)
        return t::TimePosition::fromSeconds (0.0);

    const double ppb = tl->getPixelsPerBeat();
    const double viewStartBeats = tl->getViewStartBeat().inBeats();

    // Convert event to TrackListComponent coordinates
    auto eventInTrackList = e.getEventRelativeTo (tl);
    const int globalX = eventInTrackList.x;

    // Account for header width - timeline starts after the header
    constexpr int headerWidth = 140;
    const int timelineX = globalX - headerWidth;

    // Calculate click position in beats
    const double clickBeats = viewStartBeats + (static_cast<double> (timelineX) / ppb);

    // Convert beats to TimePosition using tempo sequence
    if (clip)
    {
        auto& tempoSeq = clip->edit.tempoSequence;
        return tempoSeq.toTime (t::BeatPosition::fromBeats (clickBeats));
    }

    return t::TimePosition::fromSeconds (0.0);
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

t::TimePosition TrackClip::quantizeToGrid (t::TimePosition time, double gridSize)
{
    // Quantize in beats for musical grid alignment (Written by Claude Code)
    if (clip)
    {
        auto& tempoSeq = clip->edit.tempoSequence;
        const double beats = tempoSeq.toBeats (time).inBeats();
        const double quantizedBeats = std::round (beats / gridSize) * gridSize;
        return tempoSeq.toTime (t::BeatPosition::fromBeats (juce::jmax (0.0, quantizedBeats)));
    }

    // Fallback if no clip available
    return time;
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

    // Store initial state for potential drag
    // Note: Don't set isDragging yet - wait for mouseDrag threshold
    dragStartMousePos = e.getScreenPosition();
    originalStartTime = clip->getPosition().getStart();

    // Calculate where in the clip the user clicked (Written by Claude Code)
    // This offset lets us preserve natural drag behavior where the clip follows the cursor
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    if (tl && clip)
    {
        const double ppb = tl->getPixelsPerBeat();
        const int clickXInClip = e.getPosition().x; // X position relative to clip
        const double clickOffsetBeats = static_cast<double> (clickXInClip) / ppb;

        // Convert beats to TimeDuration using tempo sequence
        auto& tempoSeq = clip->edit.tempoSequence;
        const auto clipStartTime = clip->getPosition().getStart();
        const double clipStartBeats = tempoSeq.toBeats (clipStartTime).inBeats();
        const double clickBeats = clipStartBeats + clickOffsetBeats;

        const auto clickTime = tempoSeq.toTime (t::BeatPosition::fromBeats (clickBeats));
        clickOffsetFromStart = clickTime - clipStartTime;
    }

    // Find our track index
    if (auto* trackComp = findParentComponentOfClass<TrackComponent>())
        originalTrackIndex = trackComp->getTrackIndex();
}

void TrackClip::mouseDrag (const juce::MouseEvent& e)
{
    // Written by Claude Code
    if (!clip)
        return;

    // Check if we should activate drag based on threshold
    if (!dragThresholdExceeded)
    {
        // Use JUCE's built-in drag detection (~5px threshold)
        if (e.mouseWasDraggedSinceMouseDown())
        {
            // Threshold exceeded - activate drag mode
            isDragging = true;
            dragThresholdExceeded = true;
            setMouseCursor (juce::MouseCursor::DraggingHandCursor);
        }
        else
        {
            // Not dragging yet - allow double-click to work
            return;
        }
    }

    if (!isDragging)
        return;

    // Calculate target position with quantization
    // Get time at mouse cursor
    t::TimePosition mouseTime = mouseToTime (e);

    // Subtract click offset to get actual clip start position
    // This preserves natural drag behavior where the clip stays under cursor at grab point
    t::TimePosition targetStartTime = mouseTime - clickOffsetFromStart;

    // Clamp to prevent negative time
    if (targetStartTime.inSeconds() < 0.0)
        targetStartTime = t::TimePosition::fromSeconds (0.0);

    // Quantize the adjusted position
    t::TimePosition quantizedTime = quantizeToGrid (targetStartTime);

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
    // Reset drag threshold for next potential drag operation
    dragThresholdExceeded = false;

    if (isDragging)
    {
        isDragging = false;
        dragAlpha = 1.0f;
        setMouseCursor (juce::MouseCursor::NormalCursor);

        // Calculate final drop position
        t::TimePosition mouseTime = mouseToTime (e);

        // Subtract click offset to maintain natural drag behavior
        t::TimePosition targetStartTime = mouseTime - clickOffsetFromStart;

        // Clamp to prevent negative time
        if (targetStartTime.inSeconds() < 0.0)
            targetStartTime = t::TimePosition::fromSeconds (0.0);

        t::TimePosition quantizedTime = quantizeToGrid (targetStartTime);
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

void TrackClip::setBeingEdited (bool edited)
{
    if (isBeingEdited != edited)
    {
        isBeingEdited = edited;
        repaint(); // Redraw with highlighted border
    }
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