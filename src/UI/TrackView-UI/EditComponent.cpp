//
// Created by ikera on 4/9/2025.
//

#pragma once
#include "EditComponent.h"

#include <tracktion_engine/audio_files/formats/tracktion_FFmpegEncoderAudioFormat.h>

EditComponent::EditComponent(AppEngine& engine) : appEngine(engine) {
    //Add initial track pair
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
    int index = tracks.size();
    auto* header = new TrackHeader();
    auto* newTrack = new TrackComponent(appEngine, index);

    header->addListener(newTrack);

    newTrack->onRequestDeleteTrack = [this](int index) {
        if (index >= 0 && index < tracks.size()) {
            removeChildComponent(headers[index]);
            removeChildComponent(tracks[index]);
            headers.remove(index);
            tracks.remove(index);
            resized();
        }
    };

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

