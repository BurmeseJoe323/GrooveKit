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
     * Adds a Midi clip to a specific track (Junie).
     * @param trackIndex the index of the track to add the clip to.
     */
    void addMidiClipToTrack (int trackIndex) const;

    /**
     * Returns the first MidiClip* from a track in the edit instance (or nullptr if none) (Junie).
     * @param trackIndex the index of the track to add the clip to.
     */
    te::MidiClip* getMidiClipFromTrack (int trackIndex) const;

    /**
     * Returns all MIDI clips on the given track (Junie).
     * @param trackIndex the index of the track to add the clip to.
     */
    std::vector<te::MidiClip*> getMidiClipsFromTrack (int trackIndex) const;

    int addMidiTrack();

private:
    tracktion::engine::Edit& edit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MIDIEngine)
};
