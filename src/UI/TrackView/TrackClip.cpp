#include "TrackClip.h"

TrackClip::TrackClip() {}

TrackClip::~TrackClip() = default;

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