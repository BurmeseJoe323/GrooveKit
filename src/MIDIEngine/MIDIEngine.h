// JUNIE
#pragma once

#include "../AppEngine/TrackManager.h"
#include "tracktion_graph/tracktion_graph.h"
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

class MIDIEngine
{
public:
    explicit MIDIEngine (tracktion::engine::Edit& editRef);
    ~MIDIEngine() = default;

    /**
     * Adds a MIDI clip to a specific track (Junie).
     * @param trackIndex the index of the track to add the clip to.
     */
    void addMidiClipToTrack(int trackIndex);

    /**
     * Adds a MIDI clip at a specific beat on the given track.
     * @param trackIndex the index of the track to add a clip to.
     * @param start the start position of where the clip will be inserted.
     * @param length the length of the clip.
     */
    void addMidiClipToTrackAt(int trackIndex, te::TimePosition start, te::BeatDuration length);

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

    int addMidiTrack();

private:
    tracktion::engine::Edit& edit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIEngine)
};
