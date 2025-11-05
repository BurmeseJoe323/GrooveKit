// JUNIE
#include "TrackClip.h"
#include "TrackEditView.h"
#include "TrackComponent.h"
#include "TrackListComponent.h"

TrackClip::TrackClip (te::MidiClip* c, float pixelsPerBeat)
    : clip (c),
      pixelsPerBeat (pixelsPerBeat),
      edgeResizer (this, nullptr, juce::ResizableEdgeComponent::Edge::rightEdge)
{
    jassert (clip != nullptr);

    // Attach to the clip's ValueTree state to listen for length changes
    if (clip != nullptr)
    {
        clipState = clip->state;
        if (clipState.isValid())
            clipState.addListener (this);
    }

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

    // MidiClip time accessors
    const double lengthBeats = clip->getLengthInBeats().inBeats();
    const int w = juce::roundToInt (lengthBeats * pixelsPerBeat);

    // Keep Y/Height unchanged; only update width.
    auto b = getBounds();
    setBounds (b.withWidth (juce::jmax (1, w)));
    repaint();
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

    // Fill
    g.setColour (clipColor);
    g.fillRoundedRectangle (r, radius);

    // Border
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawRoundedRectangle (r.reduced (0.5f), radius, 1.0f);

    // Label
    g.setColour (juce::Colours::white);
    g.drawText ("MIDI Clip", getLocalBounds(), juce::Justification::centred);
}

void TrackClip::resized()
{
    constexpr int handleWidth = 8;
    edgeResizer.setBounds (getWidth() - handleWidth, 0, handleWidth, getHeight());
}

void TrackClip::mouseUp (const juce::MouseEvent& e)
{
    if (isResizing)
    {
        isResizing = false;

        auto* tl = findParentComponentOfClass<TrackListComponent>();
        if (tl != nullptr && clip != nullptr)
        {
            const double pixelsPerSecond = tl->getPixelsPerSecond();
            if (pixelsPerSecond > 0.0)
            {
                const double newLengthSecs = (double) getWidth() / pixelsPerSecond;
                clip->setLength (te::TimeDuration::fromSeconds (newLengthSecs), clip->getUndoManager());
            }
        }
    }
    // Right click: delegate context menu handling to the parent TrackComponent
    else if (e.mods.isPopupMenu())
    {
        if (onContextMenuRequested)
            onContextMenuRequested (clip);
    }

    if (auto* parent = findParentComponentOfClass<TrackComponent>())
        parent->setInterceptsMouseClicks (true, true);
}

void TrackClip::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown())
        if (e.mods.isLeftButtonDown() && onClicked)
            onClicked (clip);
}

void TrackClip::mouseDown (const juce::MouseEvent& e)
{
    if (e.originalComponent == &edgeResizer)
    {
        isResizing = true;
        resizeStartScreenX = e.getScreenX();
        originalWidth = getWidth();

        // Tell parent TrackComponent not to intercept mouse clicks
        // so this component can receive the drag events.
        if (auto* parent = findParentComponentOfClass<TrackComponent>())
            parent->setInterceptsMouseClicks (false, false);
    }
}

void TrackClip::mouseDrag (const juce::MouseEvent& e)
{
    if (isResizing)
    {
        const int dx = e.getScreenX() - resizeStartScreenX;
        const int newWidth = juce::jmax (10, originalWidth + dx);

        // Resize during a drag
        setBounds (getX(), getY(), newWidth, getHeight());
    }
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