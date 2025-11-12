#pragma once
#include <tracktion_engine/tracktion_engine.h>
#include "../DrumSamplerEngine/DrumSamplerEngineAdapter.h"
#include "../MIDIEngine/MIDIEngine.h"
#include "../PluginManager/PluginManager.h"
namespace te = tracktion::engine;

class PluginManager;
class TrackManager
{
public:
    enum class TrackType { Drum, Instrument };

    explicit TrackManager(te::Edit& editRef);
    ~TrackManager();

    int getNumTracks() const;
    te::AudioTrack* getTrack(int index);

    int  addDrumTrack();
    int  addInstrumentTrack();

    int addTrack();
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
    te::Plugin* getInstrumentPluginOnTrack (int trackIndex);
    //void clearAllTracks();

    double getClipStartSeconds (int trackIndex, int clipIndex) const;
    double getClipLengthSeconds (int trackIndex, int clipIndex) const;

    void setPluginManager(PluginManager* pm) { pluginManager = pm; }

    tracktion::engine::Plugin* insertExternalInstrument (int trackIndex,
                                                     const juce::PluginDescription& desc);

    te::Plugin* insertExternalEffect (int trackIndex,
                                  const juce::PluginDescription& desc,
                                  int insertIndex);

    tracktion::engine::Plugin* insertMorphSynth (int trackIndex);
    void clearInstrumentSlot0 (int trackIndex);


private:
    te::Edit& edit;
    PluginManager* pluginManager = nullptr;

    std::vector<TrackType> types;
    std::vector<std::unique_ptr<DrumSamplerEngineAdapter>> drumEngines;
    void syncBookkeepingToEngine();
};
