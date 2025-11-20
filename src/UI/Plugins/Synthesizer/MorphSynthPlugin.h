// ==============================================================================
// MorphSynthPlugin.h
// ------------------------------------------------------------------------------
// Tracktion Engine instrument plugin: Morph Synth
//
// Responsibilities:
//  - Owns parameters (osc types, morph, pulse, filter, envelopes, LFO, pitch).
//  - Hosts a JUCE Synthesiser with MorphVoice voices.
//  - Renders audio/MIDI and exposes parameters to Tracktion automation.
//
// Notes:
//  - See MorphSynthPlugin.cpp for parameter creation & rendering.
//  - UI binds directly to te::AutomatableParameter* members declared here.
// ==============================================================================

#pragma once

#include <tracktion_engine/tracktion_engine.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "MorphVoice.h"

namespace te = tracktion::engine;

/**
 * @brief Tracktion Engine instrument plugin for the Morph Synth.
 *
 * The plugin owns the parameter set and a JUCE Synthesiser (with MorphVoice).
 * Rendering pulls current parameter values, pushes MIDI to the synth, and
 * applies output gain. All parameters are exposed as te::AutomatableParameter
 * instances for host automation and UI attachments.
 */
class MorphSynthPlugin final : public te::Plugin,
                               private juce::Timer,
                               private MorphVoice::ParamsView
{
public:
    //==============================================================================
    // Construction & identity
    //------------------------------------------------------------------------------

    /** Stable XML/plugin type id (must match registration). */
    static inline const juce::String pluginType { "morphsynth" };

    /** Construct with Tracktion's PluginCreationInfo. */
    explicit MorphSynthPlugin (const te::PluginCreationInfo& info);

    /** Destructor. */
    ~MorphSynthPlugin() override;

    //==============================================================================
    // te::Plugin overrides
    //------------------------------------------------------------------------------

    juce::String getName() const override                   { return "MorphSynth"; }
    juce::String getPluginType() override                   { return pluginType; }
    juce::String getSelectableDescription() override        { return getName(); }
    bool isInstrument() { return true; }

    bool takesMidiInput() override                          { return true; }
    bool producesAudioWhenNoAudioInput() override           { return true; }

    void initialise   (const te::PluginInitialisationInfo&) override;
    void deinitialise () override;
    void reset        () override;

    /** Main render entry point. */
    void applyToBuffer (const te::PluginRenderContext&) override;

    //==============================================================================
    // Parameter access & persistence
    //------------------------------------------------------------------------------

    // /** Returns all parameters for automation panels, etc. */
    // te::AutomatableParameter::Array getAutomatableParameters() override;

    /** Find a parameter by ID (or nullptr). */
    te::AutomatableParameter* getParameterFromID (const juce::String& paramID);

    /** Save/load extra (non-parameter) state if needed. */
    juce::ValueTree saveToValueTree();
    void restoreFromValueTree (const juce::ValueTree&);

    /** kill all active notes immediately. */
    void stopAllNotes();

    //==============================================================================
    // Parameters (UI binds to these directly)
    //------------------------------------------------------------------------------
    // Oscillators / tone
    te::AutomatableParameter *morph = nullptr, *oscAType = nullptr, *oscBType = nullptr, *pulseWidth = nullptr;

    // Pitch
    te::AutomatableParameter *semi = nullptr, *fine = nullptr, *glide = nullptr;

    // Filter & envelopes
    te::AutomatableParameter *filterType = nullptr, *cutoff = nullptr, *resonance = nullptr;
    te::AutomatableParameter *aA = nullptr, *dA = nullptr, *sA = nullptr, *rA = nullptr;               // Amp ADSR
    te::AutomatableParameter *aF = nullptr, *dF = nullptr, *sF = nullptr, *rF = nullptr, *fEnvAmt = nullptr; // Filter ADSR + amount
    te::AutomatableParameter *keyTrack = nullptr; // optional filter key tracking (0..1)

    // LFO
    te::AutomatableParameter *lfoRate = nullptr, *lfoDepth = nullptr, *lfoTarget = nullptr;

    // Output
    te::AutomatableParameter *gain = nullptr;

private:
    //==============================================================================
    // MorphVoice::ParamsView implementation
    //------------------------------------------------------------------------------
    float get   (const juce::String& id) const override;
    int   choice(const juce::String& id) const override;

    //==============================================================================
    // Helpers
    //------------------------------------------------------------------------------

    // juce::Timer
    void timerCallback() override;

    //==============================================================================
    // State
    //------------------------------------------------------------------------------
    juce::Synthesiser synth;
    static constexpr int numVoices = 8;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MorphSynthPlugin)
};


