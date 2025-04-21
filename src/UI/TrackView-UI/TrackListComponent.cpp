//
// Created by ikera on 4/9/2025.
//

#pragma once

#include "TrackListComponent.h"

TrackListComponent::TrackListComponent (std::shared_ptr<AppEngine> engine) : appEngine (engine)
{
    //Add initial track pair
    //addNewTrack();
    setWantsKeyboardFocus (true); // setting keyboard focus?
}

TrackListComponent::~TrackListComponent() = default;

void TrackListComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}

void TrackListComponent::resized()
{
    FlexBox mainFlex;
    mainFlex.flexDirection = FlexBox::Direction::column;

    const int headerWidth = 100;
    const int trackHeight = 100;
    const int margin = 2;
    const int addButtonSpace = 30;

    int numTracks = headers.size();
    int contentH = numTracks * trackHeight + addButtonSpace;

    setSize(getParentWidth(), contentH);

    auto bounds = getLocalBounds();
    bounds.removeFromBottom (addButtonSpace); // Space for add button
    for (int i = 0; i < numTracks; i++)
    {
        // Header on left, track on right in same row
        auto row = bounds.removeFromTop (trackHeight);

        headers[i]->setBounds (row.removeFromLeft (headerWidth).reduced (margin));
        tracks[i]->setBounds (row.reduced (margin));
    }
}

void TrackListComponent::addNewTrack (int index)
{
    auto* header = new TrackHeaderComponent();
    auto* newTrack = new TrackComponent (appEngine, index);

    header->addListener (newTrack);

    // TODO : Tracks are not properly being deleted here.
    //        Should probably be in its own method.
    newTrack->onRequestDeleteTrack = [this] (int index) {
        if (index >= 0 && index < tracks.size())
        {
            removeChildComponent (headers[index]);
            removeChildComponent (tracks[index]);
            headers.remove (index);
            tracks.remove (index);
            resized();
        }
    };

    headers.add (header);
    tracks.add (newTrack);

    addAndMakeVisible (header);
    addAndMakeVisible (newTrack);
    resized();
}

void TrackListComponent::parentSizeChanged()
{
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
