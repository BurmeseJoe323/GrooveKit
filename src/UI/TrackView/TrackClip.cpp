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

void TrackClip::mouseUp (const juce::MouseEvent& e)
{
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