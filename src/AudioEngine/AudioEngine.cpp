#include "AudioEngine.h"

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

AudioEngine::AudioEngine(te::Edit& editRef)
    : edit(editRef)
{
    midiEngine = std::make_unique<MIDIEngine>(edit);
}

AudioEngine::~AudioEngine() = default;

void AudioEngine::play() {
    if (&edit)
    {
        auto& transport = edit.getTransport();
        transport.setPosition(0s);
        transport.setLoopRange(tracktion::TimeRange::between(0s, 4s));
        transport.looping = true;
        transport.play(false);
    }
}

void AudioEngine::stop() {
    if (&edit)
    {
        edit.getTransport().stop(false, false);
    }
}


