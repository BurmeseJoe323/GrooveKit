// ==============================================================================
// MorphSynthPlugin.cpp
// ------------------------------------------------------------------------------
// Parameter creation, synth setup, and rendering for MorphSynthPlugin.
// ==============================================================================

#include "MorphSynthPlugin.h"

//------------------------------------------------------------------------------
// Local sound type for JUCE Synthesiser
//------------------------------------------------------------------------------
struct MorphSound : public juce::SynthesiserSound
{
    bool appliesToNote   (int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

//------------------------------------------------------------------------------
// Parameter helpers (local to this TU)
//------------------------------------------------------------------------------
namespace
{
    using AP = te::AutomatableParameter;

    /** Create a continuous (float) parameter on the plugin. */
    static AP* addFloat (te::Plugin& plug,
                     const juce::String& id, const juce::String& name,
                     float min, float max, float def)
    {
        auto range = juce::NormalisableRange<float> (min, max);
        auto* p    = plug.addParam (id, name, range);
        if (p != nullptr)
        {
            // set default without broadcasting
            p->setParameter (def, juce::dontSendNotification);
            // or: p->setCurrentValue (def);  // if your AP uses that
        }
        return p;
    }

    /** Create a discrete (choice) parameter with text conversion. */
    static AP* addChoice (te::Plugin& plug,
                      const juce::String& id, const juce::String& name,
                      const juce::StringArray& choices, int defIndex)
    {
        jassert (choices.size() > 0);
        defIndex = juce::jlimit (0, choices.size() - 1, defIndex);

        auto range = juce::NormalisableRange<float> (0.f, (float) (choices.size() - 1), 1.f);

        auto valueToString = [choices](float v)
        {
            const int i = juce::jlimit (0, choices.size() - 1, (int) std::round (v));
            return choices[i];
        };
        auto stringToValue = [choices](const juce::String& s)
        {
            const int i = choices.indexOf (s);
            return (float) juce::jmax (0, i);
        };

        auto* p = plug.addParam (id, name, range, valueToString, stringToValue);
        if (p != nullptr)
        {
            // If your AP has helpers:
            // p->setIsDiscrete (true);
            // p->setValueNames (choices);

            p->setParameter ((float) defIndex, juce::dontSendNotification);
        }
        return p;
    }

} // namespace

//==============================================================================
// Construction / destruction
//==============================================================================

MorphSynthPlugin::MorphSynthPlugin (const te::PluginCreationInfo& info)
    : te::Plugin (info)
{
    // --- Tone
    morph      = addFloat (*this, "morph",      "Morph",       0.f,    1.f,     0.0f);
    oscAType   = addChoice(*this, "oscAType",   "Osc A",
                           juce::StringArray{ "Sine","Triangle","Saw","Pulse" }, 0);
    oscBType   = addChoice(*this, "oscBType",   "Osc B",
                           juce::StringArray{ "Sine","Triangle","Saw","Pulse" }, 2);
    pulseWidth = addFloat (*this, "pulseWidth", "Pulse Width", 0.05f,  0.95f,   0.5f);

    // --- Filter
    filterType = addChoice(*this, "filterType", "Filter",
                           juce::StringArray{ "LP","BP","HP" }, 0);
    cutoff     = addFloat (*this, "cutoff",     "Cutoff",      20.f,   20000.f, 1200.f);
    resonance  = addFloat (*this, "resonance",  "Resonance",   0.1f,   1.2f,    0.7f);

    // --- Amp envelope
    aA = addFloat (*this, "aA", "Amp A", 0.001f, 5.f, 0.01f);
    dA = addFloat (*this, "dA", "Amp D", 0.001f, 5.f, 0.12f);
    sA = addFloat (*this, "sA", "Amp S", 0.f,    1.f,  0.80f);
    rA = addFloat (*this, "rA", "Amp R", 0.001f, 5.f, 0.20f);

    // --- Filter envelope (+ amount)
    aF     = addFloat (*this, "aF", "Filt A", 0.001f, 5.f, 0.01f);
    dF     = addFloat (*this, "dF", "Filt D", 0.001f, 5.f, 0.20f);
    sF     = addFloat (*this, "sF", "Filt S", 0.f,    1.f,  0.00f);
    rF     = addFloat (*this, "rF", "Filt R", 0.001f, 5.f, 0.25f);
    fEnvAmt= addFloat (*this, "fEnvAmt", "Filt Env Amt", -1.f, 1.f, 0.5f);

    // --- Pitch / glide
    semi  = addFloat (*this, "semi",  "Semitone", -24.f, 24.f,   0.f);
    fine  = addFloat (*this, "fine",  "Fine",     -100.f, 100.f, 0.f); // cents
    glide = addFloat (*this, "glide", "Glide",     0.f,  500.f,  0.f); // ms

    // --- LFO
    lfoRate   = addFloat  (*this, "lfoRate",   "LFO Rate",  0.05f, 20.f, 5.f);
    lfoDepth  = addFloat  (*this, "lfoDepth",  "LFO Depth", 0.f,    1.f, 0.f);
    lfoTarget = addChoice (*this, "lfoTarget", "LFO Target",
                           juce::StringArray{ "Off", "Morph", "Pulse", "Cutoff", "Pitch" }, 0);

    // --- Optional key tracking
    keyTrack = addFloat (*this, "keyTrack", "KeyTrack", 0.f, 1.f, 0.f);

    // --- Output
    gain = addFloat (*this, "gain", "Output", -24.f, 6.f, 0.f);

    // --- Synth voices/sounds
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new MorphVoice());
    synth.addSound (new MorphSound());

    // Timer available for future smoothing/polling if needed.
    startTimerHz (30);

    DBG("[MorphSynth] morph=" << (morph ? morph->getCurrentValue() : -1.0f)
    << " cutoff=" << (cutoff ? cutoff->getCurrentValue() : -1.0f));
}

MorphSynthPlugin::~MorphSynthPlugin() = default;

//==============================================================================
// te::Plugin lifecycle
//==============================================================================

void MorphSynthPlugin::initialise (const te::PluginInitialisationInfo& info)
{
    const double sr       = info.sampleRate;
    const int    maxBlock = (info.blockSizeSamples > 0 ? (int) info.blockSizeSamples : 512);

    synth.setCurrentPlaybackSampleRate (sr);
    synth.setNoteStealingEnabled (true);

    // Prepare voices with engine parameters/ptrs
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<MorphVoice*> (synth.getVoice (i)))
            v->prepare (sr, maxBlock, this);
}

void MorphSynthPlugin::deinitialise()
{
    stopAllNotes();
    // synth.clearSounds();
    synth.setCurrentPlaybackSampleRate (0.0);
}

void MorphSynthPlugin::reset()
{
    stopAllNotes();
}

//==============================================================================
// Rendering
//==============================================================================

void MorphSynthPlugin::applyToBuffer (const te::PluginRenderContext& rc)
{
    auto* audio = rc.destBuffer;
    if (audio == nullptr)
        return;

    const int start   = rc.bufferStartSample;
    const int numSamp = rc.bufferNumSamples;

    // Collect MIDI
    juce::MidiBuffer midi;
    if (auto* mma = rc.bufferForMidiMessages)
        for (auto& m : *mma)
            midi.addEvent (m, 0);

    // Render synth then apply output gain
    const float g = juce::Decibels::decibelsToGain (gain ? gain->getCurrentValue() : 0.0f);
    synth.renderNextBlock (*audio, midi, start, numSamp);
    audio->applyGain (start, numSamp, g);
}

//==============================================================================
// Parameter enumeration & lookup
//==============================================================================

// juce::Array<te::AutomatableParameter*> MorphSynthPlugin::getAutomatableParameters()
// {
//     juce::Array<te::AutomatableParameter*> ps;
//
//     // Osc / morph
//     ps.add (oscAType, oscBType, morph, pulseWidth);
//
//     // Filter
//     ps.add (filterType, cutoff, resonance);
//
//     // Amp envelope
//     ps.add (aA, dA, sA, rA);
//
//     // Filter envelope (+ amount)
//     ps.add (aF, dF, sF, rF, fEnvAmt);
//
//     // Pitch / glide
//     ps.add (semi, fine, glide);
//
//     // LFO
//     ps.add (lfoTarget, lfoRate, lfoDepth);
//
//     // Optional key tracking
//     ps.add (keyTrack);
//
//     // Output
//     ps.add (gain);
//
//     return ps;
// }

te::AutomatableParameter* MorphSynthPlugin::getParameterFromID (const juce::String& pid)
{
    for (auto* p : getAutomatableParameters())
        if (p != nullptr && p->paramID == pid)
            return p;
    return nullptr;
}

//==============================================================================
// State (non-parameter)
//==============================================================================

juce::ValueTree MorphSynthPlugin::saveToValueTree()
{
    // Write into a child of the existing plugin.state node that TE serializes
    auto v = state.getOrCreateChildWithName ("MORPH_SYNTH", nullptr);
    v.removeAllProperties (nullptr); // avoid stale values

    te::AutomatableParameter* params[] = {
        oscAType, oscBType, morph, pulseWidth,
        filterType, cutoff, resonance,
        aA, dA, sA, rA,
        aF, dF, sF, rF, fEnvAmt,
        semi, fine, glide,
        lfoTarget, lfoRate, lfoDepth,
        keyTrack,
        gain
    };

    for (auto* p : params)
        if (p != nullptr)
            v.setProperty (p->paramID, p->getCurrentValue(), nullptr);

    return v; // returning child for convenience
}

void MorphSynthPlugin::restoreFromValueTree (const juce::ValueTree& /*unused*/)
{
    // Read back from the child inside plugin.state
    auto v = state.getChildWithName ("MORPH_SYNTH");
    if (! v.isValid())
        return;

    te::AutomatableParameter* params[] = {
        oscAType, oscBType, morph, pulseWidth,
        filterType, cutoff, resonance,
        aA, dA, sA, rA,
        aF, dF, sF, rF, fEnvAmt,
        semi, fine, glide,
        lfoTarget, lfoRate, lfoDepth,
        keyTrack,
        gain
    };

    for (auto* p : params)
        if (p != nullptr && v.hasProperty (p->paramID))
            p->setParameter ((float) v.getProperty (p->paramID), juce::dontSendNotification);
    // If your AP expects absolute values, setParameter is fine.
    // If it expects normalised, use setCurrentValue instead.
}

//==============================================================================
// MorphVoice::ParamsView (voices read parameter values by string key)
//==============================================================================

float MorphSynthPlugin::get (const juce::String& id) const
{
    // Tone
    if      (id == "morph")       return morph       ? morph->getCurrentValue()       : 0.f;
    else if (id == "pulseWidth")  return pulseWidth  ? pulseWidth->getCurrentValue()  : 0.f;

    // Filter
    else if (id == "cutoff")      return cutoff      ? cutoff->getCurrentValue()      : 0.f;
    else if (id == "resonance")   return resonance   ? resonance->getCurrentValue()   : 0.f;

    // Amp ADSR
    else if (id == "aA") return aA ? aA->getCurrentValue() : 0.f;
    else if (id == "dA") return dA ? dA->getCurrentValue() : 0.f;
    else if (id == "sA") return sA ? sA->getCurrentValue() : 0.f;
    else if (id == "rA") return rA ? rA->getCurrentValue() : 0.f;

    // Filter ADSR + amount
    else if (id == "aF")      return aF      ? aF->getCurrentValue()      : 0.f;
    else if (id == "dF")      return dF      ? dF->getCurrentValue()      : 0.f;
    else if (id == "sF")      return sF      ? sF->getCurrentValue()      : 0.f;
    else if (id == "rF")      return rF      ? rF->getCurrentValue()      : 0.f;
    else if (id == "fEnvAmt") return fEnvAmt ? fEnvAmt->getCurrentValue() : 0.f;

    // Pitch / glide / key tracking
    else if (id == "semi")     return semi     ? semi->getCurrentValue()     : 0.f;
    else if (id == "fine")     return fine     ? fine->getCurrentValue()     : 0.f;
    else if (id == "glide")    return glide    ? glide->getCurrentValue()    : 0.f;
    else if (id == "keyTrack") return keyTrack ? keyTrack->getCurrentValue() : 0.f;

    // LFO  **ADD THESE**
    else if (id == "lfoRate")  return lfoRate  ? lfoRate->getCurrentValue()  : 0.f;
    else if (id == "lfoDepth") return lfoDepth ? lfoDepth->getCurrentValue() : 0.f;

    // Output
    else if (id == "gain") return gain ? gain->getCurrentValue() : 0.f;

    return 0.f;
}


int MorphSynthPlugin::choice (const juce::String& id) const
{
    if      (id == "oscAType")   return oscAType   ? (int) std::round (oscAType->getCurrentValue())   : 0;
    else if (id == "oscBType")   return oscBType   ? (int) std::round (oscBType->getCurrentValue())   : 0;
    else if (id == "filterType") return filterType ? (int) std::round (filterType->getCurrentValue()) : 0;
    else if (id == "lfoTarget")  return lfoTarget  ? (int) std::round (lfoTarget->getCurrentValue())  : 0;

    return 0;
}

//==============================================================================
// Utilities
//==============================================================================

void MorphSynthPlugin::stopAllNotes()
{
    for (int ch = 1; ch <= 16; ++ch)
        synth.allNotesOff (ch, false); // false = kill immediately (no tail)
}

void MorphSynthPlugin::timerCallback() {}