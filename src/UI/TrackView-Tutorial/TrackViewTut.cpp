//
// Created by ikera on 4/14/2025.
//

#include "TrackViewTut.h"

TrackViewTut::TrackViewTut() {
}

TrackViewTut::~TrackViewTut() = default;

void TrackViewTut::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("Hello, Welcome to the TrackView Tutorial!", getLocalBounds(), juce::Justification::centred, true);
}

void TrackViewTut::resized() {

}


