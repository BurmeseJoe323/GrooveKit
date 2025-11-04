#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "MorphOsc.h"

// ==============================================================================
// MorphVoice.h
// ------------------------------------------------------------------------------
// JUCE SynthesiserVoice implementation for the Morph Synth.
//
// Responsibilities:
//  - Convert note/midi events into audio using a MorphOsc, envelopes, and SVF.
//  - Read parameter values from a lightweight ParamsView supplied by the plugin.
//  - Handle per-voice state such as envelopes, LFO phase, glide, and filter type.
//
// Notes:
//  - No logic changed in this pass; comments and structure only.
// ==============================================================================

/**
 * @brief A single voice of the Morph Synth (used by JUCE::Synthesiser).
 *
 * The voice queries parameter values through a ParamsView interface implemented
 * by the owning plugin. Audio is produced by a morphing oscillator into a
 * TPT state-variable filter, shaped by separate amp/filter ADSRs.
 */
struct MorphVoice : public juce::SynthesiserVoice
{
    //==============================================================================
    // Parameter access interface
    //------------------------------------------------------------------------------
    /**
     * @brief Read-only view of the plugin's parameters.
     *
     * Voices use string IDs to fetch current values (continuous or choice index).
     */
    struct ParamsView
    {
        virtual float get   (const juce::String& id) const = 0;
        virtual int   choice(const juce::String& id) const = 0;
    };

    //==============================================================================
    // Setup
    //------------------------------------------------------------------------------
    /**
     * @brief Prepare the voice with sample rate and block size.
     * @param sr       Sample rate in Hz.
     * @param maxBlock Maximum block size (for DSP setup).
     * @param pv       Parameter view provided by the plugin (non-owning).
     */
    void prepare (double sr, int maxBlock, ParamsView* pv)
    {
        sampleRate = sr;
        params = pv;

        osc.prepare (sr);
        ampEnv.setSampleRate (sr);
        filtEnv.setSampleRate (sr);

        juce::dsp::ProcessSpec spec { sr, (juce::uint32) maxBlock, 1 };
        svf.reset(); svf.prepare (spec);
        setFilterType (0);
    }

    //==============================================================================
    // JUCE voice overrides
    //------------------------------------------------------------------------------
    /** This voice can play any SynthesiserSound. */
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<juce::SynthesiserSound*>(s) != nullptr; }

    /** Start a note: compute base pitch (with semi/fine), reset/envelope triggers. */
    void startNote (int midiNote, float vel, juce::SynthesiserSound*, int) override
    {
        const int semiParam  = (int) params->get ("semi");
        const float fineCents = params->get ("fine"); // -100..100

        // semitone + cents -> Hz
        const double semis = (double) semiParam + fineCents / 100.0;
        baseHz   = juce::MidiMessage::getMidiNoteInHertz (midiNote + (int) std::floor (semis));
        // apply leftover cents precisely:
        const double centFrac = semis - std::floor (semis);
        baseHz *= std::pow (2.0, centFrac / 12.0);

        targetHz  = baseHz;
        currentHz = baseHz; // start at target; glide moves it after the first samples

        level = vel;
        ampEnv.noteOn();
        filtEnv.noteOn();

        // LFO phase will continue free-run (musical); leave as-is
    }

    /** Release a note and optionally tail off. */
    void stopNote (float, bool allowTailOff) override
    {
        ampEnv.noteOff();
        filtEnv.noteOff();
        if (! allowTailOff || ! ampEnv.isActive())
            clearCurrentNote();
    }

    /** Unused pitch wheel handler (kept for completeness). */
    void pitchWheelMoved (int) override {}

    /** Unused controller handler (kept for completeness). */
    void controllerMoved (int, int) override {}

    /**
     * @brief Render the next block of audio for this voice.
     *
     * Pulls all required parameters via ParamsView, applies glide and LFO, sets
     * oscillator and filter state, runs ADSRs, processes through the SVF, and
     * accumulates into the destination buffer.
     */
    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (params == nullptr) return;

        // Read parameters once
        const auto typeA   = params->choice ("oscAType");
        const auto typeB   = params->choice ("oscBType");
        const auto morph   = params->get ("morph");
        const auto pw      = params->get ("pulseWidth");
        const auto cutoff  = params->get ("cutoff");
        const auto reso    = params->get ("resonance");

        const auto aA = params->get ("aA"), dA = params->get ("dA"), sA = params->get ("sA"), rA = params->get ("rA");
        const auto aF = params->get ("aF"), dF = params->get ("dF"), sF = params->get ("sF"), rF = params->get ("rF");
        const auto glideMs  = params->get ("glide");
        const auto lfoR     = params->get ("lfoRate");
        const auto lfoD     = params->get ("lfoDepth");
        const int  lfoT     = params->choice ("lfoTarget");
        const auto keyT     = params->get ("keyTrack");
        const auto fAmt     = params->get ("fEnvAmt");

        ampEnv.setParameters ({ aA, dA, sA, rA });
        filtEnv.setParameters ({ aF, dF, sF, rF });

        setFilterType (params->choice ("filterType"));
        svf.setResonance (reso);

        osc.setTypes (typeA, typeB);
        osc.setPulseWidth (pw);

        // Glide: one-pole toward targetHz
        const double glideTimeSec = juce::jmax (0.0, (double) glideMs) * 0.001;
        const double glideCoeff   = (glideTimeSec > 0.0)
                                    ? std::exp (-1.0 / (glideTimeSec * sampleRate))
                                    : 0.0; // immediate

        // LFO increment
        lfoInc = lfoR > 0.0f ? (lfoR / sampleRate) : 0.0;

        for (int i = 0; i < num; ++i)
        {
            // Update target pitch each sample (if you want pitch LFO to act too)
            double hzNow = baseHz;

            // LFO value in [-1, 1]
            lfoPhase += lfoInc; if (lfoPhase >= 1.0) lfoPhase -= 1.0;
            const float lfo = (lfoInc > 0.0 ? std::sin (juce::MathConstants<float>::twoPi * (float) lfoPhase) : 0.0f);

            // LFO routing
            float morphMod = 0.0f;
            float pulseMod = 0.0f;
            double cutoffModMul = 1.0;  // multiplicative for musically useful cutoff modulation
            double pitchSemis = 0.0;

            switch (lfoT)
            {
                case 1: morphMod    = lfoD * 0.5f * (lfo + 1.0f); break; // 0..depth
                case 2: pulseMod    = lfoD * 0.5f * (lfo + 1.0f); break; // 0..depth
                case 3: cutoffModMul= std::pow (2.0, (double) (lfoD * lfo)); break; // +/- depth octaves-ish
                case 4: pitchSemis  = (double) (lfoD * 5.0f * lfo); break; // up to +/-5 semis at depth=1
                default: break;
            }

            // Apply pitch LFO to target pitch (then glide towards it)
            if (pitchSemis != 0.0)
                hzNow *= std::pow (2.0, pitchSemis / 12.0);

            // Update targetHz and glide currentHz toward it
            targetHz = hzNow;
            currentHz = (glideCoeff > 0.0 ? (glideCoeff * currentHz + (1.0 - glideCoeff) * targetHz)
                                          : targetHz);

            osc.setFrequency (currentHz);

            // Morph + Pulse LFO
            osc.setMorph (juce::jlimit (0.0f, 1.0f, morph + morphMod));
            osc.setPulseWidth (juce::jlimit (0.05f, 0.95f, pw + pulseMod));

            // Filter cutoff: env, keyTrack, and LFO multiplicative
            const float envF  = filtEnv.getNextSample();  // 0..1
            // key tracking: move cutoff with note pitch; 1.0 -> 1 semitone per semitone (approx)
            const double keyMul = std::pow (2.0, (keyT * (juce::jlimit (20.0, 20000.0, baseHz) / 440.0 - 1.0)) * 0.5);
            // base cutoff with env amount
            const double cBase = juce::jlimit (20.0, 20000.0, (double) cutoff * std::pow (2.0, (double) fAmt * envF));
            // apply LFO and tracking
            const double cNow  = juce::jlimit (20.0, 20000.0, cBase * cutoffModMul * keyMul);
            svf.setCutoffFrequency ((float) cNow);

            const float sig = osc.next();
            const float amp = ampEnv.getNextSample() * level;

            float x = sig;
            float* chans[] = { &x };
            juce::dsp::AudioBlock<float> b (chans, 1 /* numChannels */, 1 /* numSamples */);
            juce::dsp::ProcessContextReplacing<float> ctx (b);
            svf.process (ctx);

            const float y = x * amp;

            for (int ch = 0; ch < out.getNumChannels(); ++ch)
                out.addSample (ch, start + i, y);
        }

        if (! ampEnv.isActive())
            clearCurrentNote();
    }

private:
    //==============================================================================
    /** Map integer filter type to the JUCE TPT filter mode. */
    void setFilterType (int t)
    {
        using T = juce::dsp::StateVariableTPTFilterType;
        switch (t)
        {
            case 0: svf.setType (T::lowpass);  break;
            case 1: svf.setType (T::bandpass); break;
            case 2: svf.setType (T::highpass); break;
        }
    }

    //==============================================================================
    // Per-voice state
    //------------------------------------------------------------------------------
    ParamsView* params = nullptr;                    ///< Parameter accessor (non-owning)
    MorphOsc osc;                                    ///< Primary oscillator
    juce::ADSR ampEnv, filtEnv;                      ///< Amplitude & filter envelopes
    juce::dsp::StateVariableTPTFilter<float> svf;    ///< State-variable filter

    double lfoPhase = 0.0, lfoInc = 0.0;             ///< LFO oscillator state

    double sampleRate = 44100.0,                     ///< Voice sample rate
           baseHz     = 440.0,                       ///< Base pitch (from MIDI note + detune)
           currentHz  = 440.0,                       ///< Glide-integrated frequency
           targetHz   = 440.0;                       ///< Target frequency (pre-glide)

    float level = 1.0f;                              ///< Velocity-scaled amp
};