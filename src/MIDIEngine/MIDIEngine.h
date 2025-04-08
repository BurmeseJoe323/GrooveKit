#pragma once

#include "../AppEngine/TrackManager.h"
#include <tracktion_engine/tracktion_engine.h>


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

private:
    tracktion::engine::Edit& edit;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDIEngine)
};