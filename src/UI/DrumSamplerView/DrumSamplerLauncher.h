#pragma once
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>

class AudioEngine;
class MIDIEngine;

// Returns a owned JUCE component wrapping your JIVE-built DrumSamplerView
std::unique_ptr<juce::Component> makeDrumSamplerView(AudioEngine& audio, MIDIEngine& midi);
