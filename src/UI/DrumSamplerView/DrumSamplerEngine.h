#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @brief Abstract interface for a drum sampler engine.
 *
 * This defines the contract between the DrumSampler UI (DrumSamplerComponent,
 * DrumPadComponent, etc.) and the underlying audio backend.
 *
 * Responsibilities of an implementation:
 *  - Manage 16 (or more) drum slots/pads.
 *  - Load audio samples into specific slots.
 *  - Trigger playback of samples with a given velocity.
 *  - Control overall output volume and a simple ADSR envelope.
 *  - Provide human-readable names for each slot.
 *
 * Implementations might wrap:
 *  - A Tracktion SamplerPlugin (see DrumSamplerEngineAdapter), or
 *  - A custom JUCE sampler, or
 *  - Any other backend capable of these operations.
 */
struct DrumSamplerEngine
{
    virtual ~DrumSamplerEngine() = default;

    //==========================================================================
    /**
     * @brief Loads an audio sample into a given slot.
     *
     * @param slot Index of the drum pad/slot (typically 0–15).
     * @param file Audio file to load (usually a WAV or similar format).
     */
    virtual void loadSampleIntoSlot (int slot, const juce::File& file) = 0;

    /**
     * @brief Triggers playback of a given slot at the specified velocity.
     *
     * @param slot     Index of the drum pad/slot.
     * @param velocity Normalized velocity in the range [0.0, 1.0].
     */
    virtual void triggerSlot (int slot, float velocity) = 0;

    //==========================================================================
    /**
     * @brief Sets the overall output volume of the drum sampler.
     *
     * @param linear01 Linear gain value in [0.0, 1.0].
     */
    virtual void setVolume (float linear01) = 0; // 0..1

    /**
     * @brief Sets the ADSR envelope parameters used by the sampler.
     *
     * @param a Attack time.
     * @param d Decay time.
     * @param s Sustain level.
     * @param r Release time.
     */
    virtual void setADSR (float a, float d, float s, float r) = 0;

    //==========================================================================
    /**
     * @brief Returns a user-facing name/label for a given slot.
     *
     * Typically this will be the sample file's base name, or a default like
     * "Pad 1", "Pad 2", etc., if no sample has been assigned yet.
     *
     * @param slot Index of the drum pad/slot.
     * @return Human-readable name for the slot.
     */
    virtual juce::String getSlotName (int slot) const = 0;
};