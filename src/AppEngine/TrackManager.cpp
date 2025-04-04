#include "TrackManager.h"
#include <tracktion_engine/tracktion_engine.h>
TrackManager::TrackManager(te::Edit& editRef)
    : edit(editRef) {
}

TrackManager::~TrackManager() = default;

int TrackManager::getNumTracks() const {
    return getAudioTracks(edit).size();
}

te::AudioTrack* TrackManager::getTrack(int index) {
    auto audioTracks = getAudioTracks(edit);
    auto track = audioTracks[index];
    return track;
}

te::AudioTrack *TrackManager::addMidiTrack() {
    edit.ensureNumberOfAudioTracks(getNumTracks() + 1);
    auto* newTrack = getAudioTracks(edit).getLast();
    if (newTrack)
        newTrack->setName("MIDI Track " + getNumTracks());
    return newTrack;
}

void TrackManager::deleteTrack(int index) {
    if (index < 0 || index >= getNumTracks())
        //should probably throw error or something
        return;
    te::AudioTrack* track = getTrack(index);
    edit.deleteTrack(track);
}




