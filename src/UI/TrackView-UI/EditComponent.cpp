//
// Created by ikera on 4/9/2025.
//

#pragma once
#include "EditComponent.h"

#include <tracktion_engine/audio_files/formats/tracktion_FFmpegEncoderAudioFormat.h>

EditComponent::EditComponent() {
    //Add initial track pair
    addNewTrack();
    setWantsKeyboardFocus(true);
}

// this is the destructor
EditComponent::~EditComponent() = default;

void EditComponent::paint(juce::Graphics &g) {
    g.fillAll (juce::Colours::black);
}

void EditComponent::resized() {
    FlexBox mainFlex;
    mainFlex.flexDirection = FlexBox::Direction::column;

    const int headerWidth = 100;
    const int trackHeight = 100;
    const int margin = 2;

    auto bounds = getLocalBounds();
    bounds.removeFromBottom(30); // Space for add button

    for (int i = 0; i < headers.size(); ++i) {
        // Header on left, track on right in same row
        auto row = bounds.removeFromTop(trackHeight);

        headers[i]->setBounds(row.removeFromLeft(headerWidth).reduced(margin));
        tracks[i]->setBounds(row.reduced(margin));
    }
}

void EditComponent::addNewTrack() {
    auto* header = new TrackHeader();
    auto* newTrack = new TrackComponent();

    // Connect header's add clip button to track
    header->addListener(newTrack);

    headers.add(header);
    tracks.add(newTrack);

    addAndMakeVisible(header);
    addAndMakeVisible(newTrack);
    resized();
}

// bool EditComponent::keyPressed(const KeyPress& key) {
//     if (key == KeyPress::deleteKey) {
//         removeSelectedTracks();
//         return true;
//     }
//     return false;
// }

// void EditComponent::removeSelectedTracks() {
//     // Remove in reverse order to avoid index issues
//     for (int i = headers.size() - 1; i >= 0; --i) {
//         if (headers[i]->isSelected()) {
//             removeChildComponent(headers[i]);
//             removeChildComponent(tracks[i]);
//
//             headers.remove(i);
//             tracks.remove(i);
//         }
//     }
//     resized();
// }

