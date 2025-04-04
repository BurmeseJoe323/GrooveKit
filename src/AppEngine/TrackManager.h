#pragma once
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
class TrackManager
{
public:
    explicit TrackManager(te::Edit& editRef);
    ~TrackManager();

    // Track access
    int getNumTracks() const;
    te::AudioTrack* getTrack(int index);

    //te::AudioTrack* addAudioTrack();
    te::AudioTrack* addMidiTrack();

    void deleteTrack(int index);
    //void clearAllTracks(); // Optional

private:
    te::Edit& edit;
};
