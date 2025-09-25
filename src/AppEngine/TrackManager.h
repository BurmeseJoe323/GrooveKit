#pragma once
#include <tracktion_engine/tracktion_engine.h>
#include "../DrumSamplerEngine/DrumSamplerEngineAdapter.h"
#include "../MIDIEngine/MIDIEngine.h"
namespace te = tracktion;
class TrackManager
{
public:
    enum class TrackType { Drum, Instrument };

    explicit TrackManager(te::Edit& editRef);
    ~TrackManager();

    int getNumTracks() const;
    te::AudioTrack* getTrack(int index);
    int indexOfTrack (const te::AudioTrack* t) const;

    int  addDrumTrack();
    int  addInstrumentTrack();

    //int addTrack();
    te::AudioTrack* addMidiTrack (const juce::String& name = "MIDI Track");
    void setSelectionManager (te::SelectionManager* sm);

    void deleteTrack(int index);

    bool isDrumTrack(int index) const;
    DrumSamplerEngineAdapter* getDrumAdapter(int index);

    void muteTrack(int index);
    void setTrackMuted(int index, bool mute);
    bool isTrackMuted(int index) const;

    void soloTrack(int index);
    void setTrackSoloed(int index, bool solo);
    bool isTrackSoloed(int index) const;
    bool anyTrackSoloed() const;
    //void clearAllTracks();

    void syncBookkeepingToEngine();

private:
    te::Edit& edit;
    te::SelectionManager* selection = {};

    std::vector<TrackType> types;
    std::vector<std::unique_ptr<DrumSamplerEngineAdapter>> drumEngines;
};
