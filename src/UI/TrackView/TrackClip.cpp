// JUNIE
#include "TrackClip.h"

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
    // Right click context menu with Copy, Paste Here, and Duplicate
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu m;
        m.addItem (1, "Copy");
        m.addItem (2, "Paste Here");
        m.addItem (3, "Duplicate");
        m.addSeparator();
        m.addItem (4, "Delete");

        // Determine paste position in beats relative to this clip
        const double clipStartBeats = clip ? clip->getStartBeat().inBeats() : 0.0;
        const double localXBeats    = static_cast<double> (e.getPosition().x) / juce::jmax (1.0f, pixelsPerBeat);
        const double pasteBeats     = clipStartBeats + juce::jmax (0.0, localXBeats);

        m.showMenuAsync ({}, [safe = juce::Component::SafePointer<TrackClip>(this), pasteBeats] (int result) {
            if (safe == nullptr || safe->clip == nullptr)
                return;

            switch (result)
            {
                case 1: // Copy
                    if (safe->onCopyRequested) safe->onCopyRequested (safe->clip);
                    break;
                case 2: // Paste Here
                    if (safe->onPasteRequested) safe->onPasteRequested (safe->clip, pasteBeats);
                    break;
                case 3: // Duplicate
                    if (safe->onDuplicateRequested) safe->onDuplicateRequested (safe->clip);
                    break;
                case 4: // Delete
                    if (safe->onDeleteRequested) safe->onDeleteRequested (safe->clip);
                    break;
                default:
                    break;
            }
        });
        return;
    }

    // Left-click should open piano roll
    if (onClicked)
        onClicked(clip);
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