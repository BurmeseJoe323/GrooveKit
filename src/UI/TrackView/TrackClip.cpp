//
// Created by ikera on 4/8/2025.
//

#include "TrackClip.h"

TrackClip::TrackClip() {
    // addAndMakeVisible()
}

// this is the destructor
TrackClip::~TrackClip() = default;

void TrackClip::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::blueviolet);
}

void TrackClip::resized() {
    // this is where you modify the size of different
    // subcomponents.
}