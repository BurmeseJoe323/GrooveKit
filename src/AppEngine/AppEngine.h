#pragma once

#include <tracktion_engine/tracktion_engine.h>

#include "TrackManager.h"
#include "../MIDIEngine/MIDIEngine.h"
#include "../AudioEngine/AudioEngine.h"

class AppEngine
{
public:
    AppEngine();
    ~AppEngine();

    void createOrLoadEdit();
    void play();
    void stop();

    void addMidiTrack();
    void addMidiClipToTrack(int trackIndex = 0);
    int getNumTracks();

private:
    std::unique_ptr<tracktion::engine::Engine> engine;
    std::unique_ptr<tracktion::engine::Edit> edit;

    std::unique_ptr<MIDIEngine> midiEngine;
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<TrackManager> trackManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppEngine)
};