#pragma once

#include "TrackListComponent.h"

#include <juce_graphics/fonts/harfbuzz/hb-ot-head-table.hh>

TrackListComponent::TrackListComponent(std::shared_ptr<AppEngine> engine) : appEngine(engine),
                                                                            playhead(engine->getEdit(),
                                                                                     engine->getEditViewState())
{
    //Add initial track pair
    //addNewTrack();
    setWantsKeyboardFocus (true); // setting keyboard focus?
    addAndMakeVisible(playhead);
    playhead.setAlwaysOnTop(true);
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

    const int headerWidth = 140;
    const int trackHeight = 100;
    const int margin = 2;
    const int addButtonSpace = 30;

    int numTracks = headers.size();
    int contentH = numTracks * trackHeight + addButtonSpace;

    // Set the size to either default to the parent's height if the content height isn't tall enough
    setSize(getParentWidth(), contentH > getParentHeight() ? contentH : getParentHeight());

    auto bounds = getLocalBounds();
    bounds.removeFromBottom (addButtonSpace); // Space for add button
    for (int i = 0; i < numTracks; i++)
    {
        // Header on left, track on right in same row
        auto row = bounds.removeFromTop (trackHeight);

        headers[i]->setBounds (row.removeFromLeft (headerWidth).reduced (margin));
        tracks[i]->setBounds (row.reduced (margin));
    }

    // Set bounds for playhead
    playhead.setBounds (getLocalBounds().withTrimmedLeft(headerWidth));
}

void TrackListComponent::addNewTrack (int index)
{
    auto* header = new TrackHeaderComponent();
    auto* newTrack = new TrackComponent(appEngine, index);

    header->addListener(newTrack);

    // TODO : Tracks are not properly being deleted here.
    //        Should probably be in its own method.
    newTrack->onRequestDeleteTrack = [this] (int index) {
        if (index >= 0 && index < tracks.size())
        {
            removeChildComponent (headers[index]);
            removeChildComponent (tracks[index]);
            headers.remove (index);
            tracks.remove (index);
            appEngine->deleteMidiTrack(index);
            updateTrackIndexes();
            resized();
        }
    };

    newTrack->onRequestOpenPianoRoll = [this] (int index) {
        if (index >= 0 && index < tracks.size())
        {
            if (pianoRollWindow == nullptr) {
                pianoRollWindow = std::make_unique<PianoRollWindow>();
                addAndMakeVisible(pianoRollWindow.get());
                pianoRollWindow->addToDesktop(pianoRollWindow->getDesktopWindowStyleFlags());
                pianoRollWindow->toFront(true);
                pianoRollWindow->centreWithSize(pianoRollWindow->getWidth(), pianoRollWindow->getHeight());
            } else {
                pianoRollWindow->setVisible(true);
                pianoRollWindow->toFront(true);
            }
        }
    };

    headers.add (header);
    tracks.add (newTrack);

    addAndMakeVisible (header);
    addAndMakeVisible (newTrack);

    updateTrackIndexes();

    const int uiIndex = tracks.indexOf(newTrack);
    header->setMuted(appEngine->isTrackMuted(uiIndex));
    header->setSolo (appEngine->isTrackSoloed(uiIndex));
    refreshSoloVisuals();

    resized();
}

void TrackListComponent::parentSizeChanged()
{
    resized();
}

void TrackListComponent::updateTrackIndexes()
{
    for (int i = 0; i < tracks.size(); ++i)
        tracks[i]->setTrackIndex(i);
}

void TrackListComponent::refreshSoloVisuals()
{
    const bool anySolo = appEngine->anyTrackSoloed();
    for (int i = 0; i < headers.size(); ++i)
    {
        const bool thisSolo = appEngine->isTrackSoloed(i);
        headers[i]->setDimmed(anySolo && !thisSolo);
    }
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
