#pragma once
#include <tracktion_engine/tracktion_engine.h>
#include "../DrumSamplerEngine/DrumSamplerEngineAdapter.h"
#include "../MIDIEngine/MIDIEngine.h"
#include "../PluginManager/PluginManager.h"
namespace te = tracktion::engine;

class PluginManager;

/**
 * @brief Manages track lifecycle, type identification, and mute/solo state for drum and instrument tracks.
 *
 * TrackManager provides a specialized layer on top of Tracktion Engine's AudioTrack system,
 * distinguishing between two track types:
 *  - **Drum tracks**: Backed by DrumSamplerEngineAdapter, use `gk_isDrum` property
 *  - **Instrument tracks**: Standard MIDI tracks with plugin instruments
 *
 * Architecture:
 *  - Maintains parallel bookkeeping vectors (types[], drumEngines[]) indexed by track position
 *  - Synchronizes with Tracktion Edit's track list via syncBookkeepingToEngine()
 *  - Uses Tracktion's ValueTree properties for persistent track metadata (`gk_isDrum`)
 *  - Owned by AppEngine for centralized track management
 *
 * Track Type System:
 *  - Drum tracks are identified by `gk_isDrum = true` property in track ValueTree
 *  - DrumSamplerEngineAdapter is created for each drum track, mapping 16 pads to MIDI notes 36-51
 *  - Instrument tracks use standard Tracktion plugin system (MorphSynth, external plugins)
 *
 * Usage:
 *  - Call addDrumTrack() or addInstrumentTrack() to create typed tracks
 *  - Use isDrumTrack() to query track type
 *  - Access drum sampler via getDrumAdapter() for drum tracks
 *  - Mute/solo operations update Tracktion track state and notify listeners
 */
class TrackManager
{
public:
    //==============================================================================
    // Type Definitions

    enum class TrackType { Drum, Instrument };

    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the TrackManager.
     *
     * @param editRef Reference to Tracktion Edit for track access
     */
    explicit TrackManager(te::Edit& editRef);

    /** Destructor. */
    ~TrackManager();

    //==============================================================================
    // Track Access

    /**
     * @brief Returns the total number of audio tracks in the Edit.
     *
     * @return Number of tracks
     */
    int getNumTracks() const;

    /**
     * @brief Returns a pointer to the track at the given index.
     *
     * @param index Track index (0-based)
     * @return Pointer to AudioTrack, or nullptr if index invalid
     */
    te::AudioTrack* getTrack(int index);

    //==============================================================================
    // Track Creation / Deletion

    /**
     * @brief Creates a new drum track with DrumSamplerEngineAdapter.
     *
     * Sets `gk_isDrum = true` property and loads default drum samples.
     *
     * @return Index of newly created drum track
     */
    int addDrumTrack();

    /**
     * @brief Creates a new instrument track for MIDI/plugin instruments.
     *
     * @return Index of newly created instrument track
     */
    int addInstrumentTrack();

    /**
     * @brief Creates a new generic track (deprecated, use typed methods).
     *
     * @return Index of newly created track
     */
    int addTrack();

    /**
     * @brief Deletes the track at the given index.
     *
     * Updates bookkeeping vectors and removes from Tracktion Edit.
     *
     * @param index Track index to delete
     */
    void deleteTrack(int index);

    //==============================================================================
    // Track Type Identification

    /**
     * @brief Checks if a track is a drum track.
     *
     * Queries the `gk_isDrum` property in the track's ValueTree state.
     *
     * @param index Track index
     * @return true if drum track, false if instrument track or invalid index
     */
    bool isDrumTrack(int index) const;

    /**
     * @brief Returns the DrumSamplerEngineAdapter for a drum track.
     *
     * @param index Track index
     * @return Pointer to DrumSamplerEngineAdapter, or nullptr if not a drum track
     */
    DrumSamplerEngineAdapter* getDrumAdapter(int index);

    //==============================================================================
    // Mute / Solo State

    /**
     * @brief Toggles mute state for the given track.
     *
     * @param index Track index
     */
    void muteTrack(int index);

    /**
     * @brief Sets the mute state for the given track.
     *
     * @param index Track index
     * @param mute true to mute, false to unmute
     */
    void setTrackMuted(int index, bool mute);

    /**
     * @brief Returns whether the track is muted.
     *
     * @param index Track index
     * @return true if muted, false otherwise
     */
    bool isTrackMuted(int index) const;

    /**
     * @brief Toggles solo state for the given track.
     *
     * @param index Track index
     */
    void soloTrack(int index);

    /**
     * @brief Sets the solo state for the given track.
     *
     * @param index Track index
     * @param solo true to solo, false to unsolo
     */
    void setTrackSoloed(int index, bool solo);

    /**
     * @brief Returns whether the track is soloed.
     *
     * @param index Track index
     * @return true if soloed, false otherwise
     */
    bool isTrackSoloed(int index) const;

    /**
     * @brief Returns whether any track in the Edit is currently soloed.
     *
     * @return true if at least one track is soloed
     */
    bool anyTrackSoloed() const;

    //==============================================================================
    // Plugin / Instrument Access

    /**
     * @brief Returns the first instrument plugin on a track.
     *
     * Searches for the first plugin at slot 0 (instrument position).
     *
     * @param trackIndex Track index
     * @return Pointer to Plugin, or nullptr if no instrument found
     */
    te::Plugin* getInstrumentPluginOnTrack (int trackIndex);

    //==============================================================================
    // Clip Information

    /**
     * @brief Returns the start time of a clip in seconds.
     *
     * @param trackIndex Track index
     * @param clipIndex Clip index on that track
     * @return Start time in seconds
     */
    double getClipStartSeconds (int trackIndex, int clipIndex) const;

    /**
     * @brief Returns the length of a clip in seconds.
     *
     * @param trackIndex Track index
     * @param clipIndex Clip index on that track
     * @return Clip length in seconds
     */
    double getClipLengthSeconds (int trackIndex, int clipIndex) const;

    void setPluginManager(PluginManager* pm) { pluginManager = pm; }

    tracktion::engine::Plugin* insertExternalInstrument (int trackIndex,
                                                     const juce::PluginDescription& desc);

    te::Plugin* insertExternalEffect (int trackIndex,
                                  const juce::PluginDescription& desc,
                                  int insertIndex);

    tracktion::engine::Plugin* insertMorphSynth (int trackIndex);
    void clearInstrumentSlot0 (int trackIndex);
    void clearFxInsertSlot (int trackIndex, int slotIndex);
    int getFxInsertBaseIndex (int trackIndex) const;


private:
    //==============================================================================
    // Member Variables

    te::Edit& edit; ///< Reference to Tracktion Edit (not owned)
    PluginManager* pluginManager = nullptr;

    std::vector<TrackType> types; ///< Parallel array tracking track types by index
    std::vector<std::unique_ptr<DrumSamplerEngineAdapter>> drumEngines; ///< Drum samplers for drum tracks (nullptrs for instrument tracks)

    //==============================================================================
    // Internal Methods

    /**
     * @brief Synchronizes bookkeeping vectors with Tracktion Edit's track list.
     *
     * Rebuilds types[] and drumEngines[] to match current Edit state.
     * Called after track addition/deletion to maintain consistency.
     */
    void syncBookkeepingToEngine();
};
