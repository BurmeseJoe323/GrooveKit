#pragma once

#include "../AppEngine/TrackManager.h"
#include "tracktion_graph/tracktion_graph.h"
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;
namespace t = tracktion;

/**
 * @brief MIDI clip creation and retrieval engine for managing MIDI content on tracks.
 *
 * MIDIEngine provides a simplified API for creating and accessing MIDI clips within
 * Tracktion Engine's Edit. It abstracts away the complexity of Tracktion's clip system,
 * providing convenient methods for:
 *  - Creating MIDI clips at specific beat positions with defined lengths
 *  - Retrieving individual or all MIDI clips from a track
 *  - Adding new MIDI tracks to the Edit
 *
 * Architecture:
 *  - Owned by AudioEngine (created during AudioEngine construction)
 *  - Operates directly on Tracktion Edit's track and clip structures
 *  - Uses Tracktion's TimePosition and BeatDuration types for timing
 *  - Filters clips by type (te::MidiClip) when retrieving from tracks
 *
 * Usage:
 *  - Call addMidiClipToTrack() for default 4-beat clips at playhead
 *  - Call addMidiClipToTrackAt() for precise positioning and length
 *  - Use getMidiClipsFromTrack() to access all clips for editing (e.g., piano roll)
 *  - Clips are automatically added to Tracktion Edit's undo history
 */
class MIDIEngine
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the MIDIEngine.
     *
     * @param editRef Reference to Tracktion Edit for clip operations
     */
    explicit MIDIEngine(te::Edit& editRef);

    /** Default destructor. */
    ~MIDIEngine() = default;

    //==============================================================================
    // MIDI Clip Creation

    /**
     * @brief Adds a MIDI clip at the current playhead position with default 4-beat length.
     *
     * Creates a new MIDIClip on the specified track using Tracktion's insertNewClip() method.
     * Clip starts at current transport position with 4-beat duration.
     *
     * @param trackIndex Index of track to add clip to (0-based)
     * @return true if successful, false if track index invalid
     */
    bool addMidiClipToTrack(int trackIndex);

    /**
     * @brief Adds a MIDI clip at a specific beat position with specified length.
     *
     * Creates a precisely positioned MIDIClip with custom start time and duration.
     * Useful for programmatic clip creation, paste operations, and importing.
     *
     * @param trackIndex Index of track to add clip to (0-based)
     * @param start Start position in timeline (TimePosition)
     * @param length Duration of the clip (BeatDuration)
     * @return true if successful, false if track index invalid
     */
    bool addMidiClipToTrackAt (int trackIndex, t::TimePosition start, t::BeatDuration length);

    //==============================================================================
    // MIDI Clip Retrieval

    /**
     * @brief Returns the first MidiClip on the given track.
     *
     * Searches through the track's clip list and returns the first MIDIClip found.
     * Useful for tracks with single clips or when only the first clip matters.
     *
     * @param trackIndex Index of track to query
     * @return Pointer to first MidiClip, or nullptr if none found
     */
    te::MidiClip* getMidiClipFromTrack(int trackIndex);

    /**
     * @brief Returns all MIDI clips on the given track.
     *
     * Filters the track's clip list and returns only MIDIClip objects.
     * Used by piano roll editor and clip management UI to access all MIDI content.
     *
     * @param trackIndex Index of track to query
     * @return Array of MidiClip pointers (empty if no clips)
     */
    juce::Array<te::MidiClip*> getMidiClipsFromTrack(int trackIndex);

    //==============================================================================
    // Track Creation

    /** Import a MIDI file as a single MidiClip on the given track.
        @param midiFile   The .mid file to import
        @param trackIndex Index of the target track in the Edit's track list
        @param destStart  Where to place the start of the imported clip in the Edit
        @return true on success, false if something failed (bad file, bad track, etc.)
    */
    bool importMidiFileToTrack (const juce::File& midiFile,
                                int trackIndex,
                                t::TimePosition destStart);

    /**
     * @brief Adds a new MIDI track to the Edit (deprecated).
     *
     * Creates a basic AudioTrack for MIDI content. Prefer using TrackManager's
     * addDrumTrack() or addInstrumentTrack() for typed track creation.
     *
     * @return Index of newly created track
     */
    int addMidiTrack();

private:
    //==============================================================================
    // Member Variables

    te::Edit& edit; ///< Reference to Tracktion Edit (not owned)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIEngine)
};
