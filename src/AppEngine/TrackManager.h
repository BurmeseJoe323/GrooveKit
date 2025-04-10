#pragma once
#include <tracktion_engine/tracktion_engine.h>
#include "../MIDIEngine/MIDIEngine.h"
namespace te = tracktion;
class TrackManager
{
public:
    explicit TrackManager(te::Edit& editRef);
    ~TrackManager();

    int getNumTracks() const;
    te::AudioTrack* getTrack(int index);

    int addTrack();

    void deleteTrack(int index);
    //void clearAllTracks();

private:
    te::Edit& edit;
};
