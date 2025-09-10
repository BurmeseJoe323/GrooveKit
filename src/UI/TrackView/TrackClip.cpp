#include "TrackClip.h"

TrackClip::TrackClip() {}

TrackClip::~TrackClip() = default;

void TrackClip::paint (juce::Graphics& g)
{
    g.fillAll (clipColor);
    g.setColour (juce::Colours::white);
    g.drawText ("MIDI Clip", getLocalBounds(), juce::Justification::centred);
}

void TrackClip::resized() {}

void TrackClip::setColor (juce::Colour newColor)
{
    clipColor = newColor;
    repaint();
}