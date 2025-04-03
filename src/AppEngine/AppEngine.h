#pragma once

#include <tracktion_engine/tracktion_engine.h>
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

private:
    std::unique_ptr<tracktion::engine::Engine> engine;
    std::unique_ptr<tracktion::engine::Edit> edit;

    std::unique_ptr<MIDIEngine> midiEngine;
    std::unique_ptr<AudioEngine> audioEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppEngine)
};