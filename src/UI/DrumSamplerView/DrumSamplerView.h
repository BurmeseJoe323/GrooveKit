#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <jive_layouts/jive_layouts.h>
#include <jive_style_sheets/jive_style_sheets.h>

class MIDIEngine;      // forward-declare your engines
class AudioEngine;

class DrumSamplerView : public juce::Component{
    public:
    DrumSamplerView(AudioEngine& audio, MIDIEngine& midi);

private:
    AudioEngine& audio;
    MIDIEngine&  midi;

    juce::ValueTree layoutTree;
    juce::var       styleJson;
    std::unique_ptr<jive::GuiItem> root;    // JIVE-built GUI item

    void wirePads();
    void wireControls();
    void resized() override;
};

