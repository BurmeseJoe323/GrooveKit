// ==============================================================================
// MorphOsc.h
// ------------------------------------------------------------------------------
// Lightweight morphing oscillator used by the Morph Synth.
//
// Features:
//  - Two selectable waveform types (A and B)
//  - Per-sample morph blend between A and B (0..1)
//  - Pulse width for pulse waveform (clamped 0.05..0.95)
//  - Simple phase-accumulator oscillator with normalized phase [0, 1)
//
// Notes:
//  - Call prepare(sampleRate) before use.
//  - setFrequency(hz) updates the phase increment; safe to call per-sample.
//  - next() advances phase and returns the current sample.
// ==============================================================================

#pragma once
#include <juce_dsp/juce_dsp.h>

/**
 * @brief Simple morphing oscillator (two wave sources crossfaded by @c morph).
 *
 * Wave types are identified by integer IDs (kept for compatibility) and an
 * optional enum for readability. See Type below.
 */
struct MorphOsc
{
    //==============================================================================
    // Types
    //------------------------------------------------------------------------------

    /** Optional readable IDs for wave selection (values match existing int API). */
    enum Type : int
    {
        Sine     = 0,
        Triangle = 1,
        Saw      = 2,
        Pulse    = 3
    };

    //==============================================================================
    // Lifecycle
    //------------------------------------------------------------------------------

    /**
     * @brief Prepare the oscillator with a given sample rate.
     * @param sr Sample rate in Hz.
     *
     * Must be called before generating audio.
     */
    void prepare (double sr)
    {
        sampleRate = sr;
        setFrequency (freqHz); // refresh phaseInc with the new sample rate
    }

    //==============================================================================
    // Parameters
    //------------------------------------------------------------------------------

    /**
     * @brief Set oscillator frequency in Hz.
     *
     * Safe to call frequently; computes phase increment = hz / sampleRate.
     */
    void setFrequency (double hz)
    {
        freqHz   = hz;
        phaseInc = hz / sampleRate;
    }

    /**
     * @brief Set morph amount between waveform A and B.
     * @param m 0.0 = A only, 1.0 = B only.
     */
    void setMorph (float m)
    {
        morph = juce::jlimit (0.0f, 1.0f, m);
    }

    /**
     * @brief Select waveform types using integer IDs (compatible API).
     * @param a Wave type for "A" (see Type enum).
     * @param b Wave type for "B" (see Type enum).
     */
    void setTypes (int a, int b)
    {
        typeA = a;
        typeB = b;
    }

    /**
     * @brief Overload for readable enum usage (non-breaking).
     */
    void setTypes (Type a, Type b) { setTypes ((int) a, (int) b); }

    /**
     * @brief Set pulse width for pulse waveform.
     * @param pw Duty cycle in [0.05 .. 0.95].
     *
     * Only affects the Pulse type; clamped to avoid silent/numerically unstable
     * extremes.
     */
    void setPulseWidth (float pw)
    {
        pulseWidth = juce::jlimit (0.05f, 0.95f, pw);
    }

    //==============================================================================
    // Synthesis
    //------------------------------------------------------------------------------

    /**
     * @brief Generate the next sample and advance phase.
     * @return The mixed sample from wave A and wave B according to @c morph.
     */
    float next()
    {
        // advance phase in [0, 1)
        phase += phaseInc;
        if (phase >= 1.0)
            phase -= 1.0;

        const float wa = oscSample (typeA);
        const float wb = oscSample (typeB);
        return wa * (1.0f - morph) + wb * morph;
    }

private:
    //==============================================================================
    // Helpers
    //------------------------------------------------------------------------------

    /**
     * @brief Render a single-sample from the given wave @p type using current phase.
     *
     * Phase is normalized [0, 1). All outputs are in [-1, 1].
     */
    float oscSample (int type) const
    {
        const float x = (float) phase;

        switch (type)
        {
            case Sine:
                // Sine: sin(2πx)
                return std::sin (juce::MathConstants<float>::twoPi * x);

            case Triangle:
            {
                // Triangle: bipolar, linear slopes. Classic folded saw formula:
                // t = 2 * |2*(x - floor(x + 0.5))| - 1  in [-1, 1]
                const float t = 2.0f * std::abs (2.0f * (x - std::floor (x + 0.5f))) - 1.0f;
                return t;
            }

            case Saw:
                // Saw: maps [0,1) to [-1,1), linear rise
                return 2.0f * x - 1.0f;

            case Pulse:
                // Pulse: high for first 'pulseWidth' portion of cycle, else low
                return (x < pulseWidth) ? 1.0f : -1.0f;

            default:
                return 0.0f;
        }
    }

    //==============================================================================
    // State
    //------------------------------------------------------------------------------

    // Audio/timebase
    double sampleRate = 44100.0;
    double freqHz     = 440.0;
    double phase      = 0.0;   // normalized [0, 1)
    double phaseInc   = 0.0;   // per-sample increment (freqHz / sampleRate)

    // Parameters
    float morph       = 0.0f;  // 0..1 (A->B)
    float pulseWidth  = 0.5f;  // duty for Pulse

    // Wave selections (defaults: A=Sine, B=Saw to match original)
    int typeA         = (int) Sine;
    int typeB         = (int) Saw;
};