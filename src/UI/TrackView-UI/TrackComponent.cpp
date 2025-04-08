//
// Created by ikera on 4/6/2025.
//

#include "TrackComponent.h"

#include <tracktion_engine/utilities/tracktion_Identifiers.h>


TrackComponent::TrackComponent() {
    addAndMakeVisible(trackHeader);
}

TrackComponent::~TrackComponent() = default;

void TrackComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::grey);
    // TODO: select tracks, Take a look at example from Tracktion
}

void TrackComponent::resized() {
    auto area = getLocalBounds();

    const int margin = 5;
    area.reduce(margin, margin);

    auto trackHeaderHeight = 100;
    trackHeader.setBounds(area.removeFromLeft(trackHeaderHeight));
}


