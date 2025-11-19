#pragma once

#include "../AppEngine/TrackManager.h"
#include "tracktion_graph/tracktion_graph.h"
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;
namespace t = tracktion;

class MIDIEngine
{
public:
    explicit MIDIEngine(te::Edit& editRef);
    ~MIDIEngine() = default;

    /**
     * Adds a MIDI clip at a specific beat on the given track.
     * @param trackIndex the index of the track to add a clip to.
     * @param start the start position of where the clip will be inserted.
     * @param length the length of the clip.
     * @return true if successful; false otherwise.
     */
    bool addMidiClipToTrack(int trackIndex);
    /**
     * Adds a MIDI clip at a specific beat on the given track.
     * @param trackIndex the index of the track to add a clip to.
     * @param start the start position of where the clip will be inserted.
     * @param length the length of the clip.
     * @return true if successful; false otherwise.
     */
    bool addMidiClipToTrackAt (int trackIndex, t::TimePosition start, t::BeatDuration length);


    /**
     * Returns the first MidiClip* from a track in the edit instance (or nullptr if none) (Junie).
     * @param trackIndex the index of the track.
     */
    te::MidiClip *getMidiClipFromTrack(int trackIndex);

    /**
     * Returns all MIDI clips on the given track (Junie).
     * @param trackIndex the index of the track.
     */
    juce::Array<te::MidiClip*> getMidiClipsFromTrack(int trackIndex);

    /** Import a MIDI file as a single MidiClip on the given track.
        @param midiFile   The .mid file to import
        @param trackIndex Index of the target track in the Edit's track list
        @param destStart  Where to place the start of the imported clip in the Edit
        @return true on success, false if something failed (bad file, bad track, etc.)
    */
    bool importMidiFileToTrack (const juce::File& midiFile,
                                int trackIndex,
                                t::TimePosition destStart);

    int addMidiTrack();

private:
    te::Edit& edit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIEngine)
};
