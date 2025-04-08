#pragma once
#include <tracktion_engine/tracktion_engine.h>
#include "../MIDIEngine/MIDIEngine.h"
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
    te::AudioTrack* addTrack();

    void deleteTrack(int index);
    //void clearAllTracks();

private:
    te::Edit& edit;
};
