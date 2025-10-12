#include "TrackClip.h"

TrackClip::TrackClip(te::MidiClip* c, float pixelsPerBeat) : clip(c), pixelsPerBeat(pixelsPerBeat)
{
    jassert(clip != nullptr);
    clip->addListener(this);
    updateSizeFromClip();
}

TrackClip::~TrackClip()
{
}

void TrackClip::updateSizeFromClip()
{
    if (!clip) return;

    // Midiclip times are in seconds/beats
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
    auto r = getLocalBounds().toFloat();
    const float radius = 8.0f;

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

void TrackClip::setColor (juce::Colour newColor)
{
    clipColor = newColor;
    repaint();
}