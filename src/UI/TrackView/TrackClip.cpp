// JUNIE
#include "TrackClip.h"
#include "TrackEditView.h"

TrackClip::TrackClip(te::MidiClip* c, float pixelsPerBeat) : clip(c), pixelsPerBeat(pixelsPerBeat)
{
    jassert(clip != nullptr);

    // Attach to the clip's ValueTree state to listen for length changes
    if (clip != nullptr)
    {
        clipState = clip->state;
        if (clipState.isValid())
            clipState.addListener(this);
    }

    updateSizeFromClip();
}

TrackClip::~TrackClip()
{
    if (clipState.isValid())
        clipState.removeListener(this);
}

void TrackClip::updateSizeFromClip()
{
    if (!clip) return;

    // MidiClip time accessors
    const double lengthBeats = clip->getLengthInBeats().inBeats();
    const int w = juce::roundToInt(lengthBeats * pixelsPerBeat);

    // Keep Y/Height unchanged; only update width.
    auto b = getBounds();
    setBounds(b.withWidth(juce::jmax(1, w)));
    repaint();
}

void TrackClip::setPixelsPerBeat(float ppb) {
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

void TrackClip::resized() {}

void TrackClip::mouseUp (const juce::MouseEvent& e)
{
    // Right click: delegate context menu handling to the parent TrackComponent
    if (e.mods.isPopupMenu())
        if (onContextMenuRequested)
            onContextMenuRequested (clip);
}

void TrackClip::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.mods.isLeftButtonDown())
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
        juce::MessageManager::callAsync ([safeThis]
        {
            if (safeThis != nullptr)
                safeThis->updateSizeFromClip();
        });
    }
}