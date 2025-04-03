#pragma once
#include "../MIDIEngine/MIDIEngine.h"

class AudioEngine
{
public:
    AudioEngine(tracktion::engine::Edit& editRef);
    ~AudioEngine();

    void play();
    void stop();

private:
    tracktion::engine::Edit& edit;
    std::unique_ptr<MIDIEngine> midiEngine;

};
