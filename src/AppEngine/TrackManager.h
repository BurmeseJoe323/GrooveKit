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

    void muteTrack(int index);
    void setTrackMuted(int index, bool mute);
    bool isTrackMuted(int index) const;

    void soloTrack(int index);
    void setTrackSoloed(int index, bool solo);
    bool isTrackSoloed(int index) const;
    bool anyTrackSoloed() const;
    //void clearAllTracks();

private:
    te::Edit& edit;
};
