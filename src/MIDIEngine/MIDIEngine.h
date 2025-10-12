// JUNIE
#pragma once

#include "../AppEngine/TrackManager.h"
#include <tracktion_engine/tracktion_engine.h>

#include "tracktion_graph/tracktion_graph.h"

namespace te = tracktion;

class MIDIEngine
{
public:
    explicit MIDIEngine(tracktion::engine::Edit& editRef);
    ~MIDIEngine() = default;

    /**
     * Adds a Midi clip to a specific track
     * @param trackIndex the index of the track to add the clip to.
     */
    void addMidiClipToTrack(int trackIndex);

    /**
     * Returns the first MidiClip* from a track in the edit instance (or nullptr if none)
     */
    te::MidiClip* getMidiClipFromTrack(int trackIndex);

    /**
     * Returns all MIDI clips on the given track.
     */
    std::vector<te::MidiClip*> getMidiClipsFromTrack(int trackIndex);

    int addMidiTrack();

private:
    tracktion::engine::Edit& edit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDIEngine)
};
