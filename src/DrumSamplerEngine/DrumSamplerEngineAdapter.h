#pragma once

#include "../UI/DrumSamplerView/DrumSamplerEngine.h"
#include <tracktion_engine/tracktion_engine.h>
#include <tracktion_graph/tracktion_graph.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <utility>

namespace te = tracktion::engine;

/**
 * @brief Converts drum pad index (0–15) to MIDI note number (36–51).
 *
 * Maps pad layout to General MIDI drum map range:
 *  - Pad 0 → MIDI note 36 (Kick)
 *  - Pad 15 → MIDI note 51 (Ride cymbal)
 *
 * @param padIndex Pad index (0–15, clamped if out of range).
 * @return MIDI note number in the range [36, 51].
 */
static inline int padToMidiNote (int padIndex)
{
    return 36 + juce::jlimit (0, 15, padIndex);
}

/**
 * @brief Adapts Tracktion Engine's SamplerPlugin to GrooveKit's DrumSamplerEngine interface.
 *
 * DrumSamplerEngineAdapter bridges the gap between GrooveKit's abstract DrumSamplerEngine
 * interface (used by DrumSamplerView UI) and Tracktion Engine's concrete SamplerPlugin.
 *
 * Responsibilities:
 *  - Create or find a te::SamplerPlugin on the given te::AudioTrack.
 *  - Manage a 16-pad drum sampler mapped to MIDI notes 36–51.
 *  - Load WAV files into specific note slots.
 *  - Trigger samples via live MIDI injection into the track.
 *  - Keep per-pad display names for the UI.
 *
 * Ownership:
 *  - Holds references to te::Engine and te::AudioTrack (not owned).
 *  - Holds a raw pointer to a te::SamplerPlugin created/owned by the Edit.
 *
 * Usage:
 *  - Created by TrackManager when a drum track is added.
 *  - DrumSamplerView calls loadSampleIntoSlot() to assign WAV files.
 *  - DrumPadComponent calls triggerSlot() to play a pad.
 */
class DrumSamplerEngineAdapter : public DrumSamplerEngine
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs an adapter bound to a specific Tracktion audio track.
     *
     * Attempts to find an existing SamplerPlugin on the track. If none is found,
     * it creates a new SamplerPlugin and inserts it at index 0 in the track's
     * plugin list.
     *
     * @param engine Tracktion Engine instance (not owned).
     * @param track Audio track to attach the sampler to (not owned).
     */
    DrumSamplerEngineAdapter (te::Engine& engine, te::AudioTrack& track);

    /** Default virtual destructor. */
    ~DrumSamplerEngineAdapter() override = default;

    //==============================================================================
    // DrumSamplerEngine Overrides

    /**
     * @brief Loads a sample file into a drum pad slot.
     *
     * Assigns the sample to the pad's corresponding MIDI note (36–51) in the
     * underlying SamplerPlugin. If a sound is already mapped to this note,
     * it reuses the slot; otherwise a new sound is created.
     *
     * @param slot Drum pad index (0–15).
     * @param file WAV file to load into the pad.
     */
    void loadSampleIntoSlot (int slot, const juce::File& file) override;

    /**
     * @brief Triggers playback of a pad at a given velocity.
     *
     * Sends a MIDI note-on followed by an automatic note-off after a short
     * delay (80 ms) by injecting MIDI into the associated track.
     *
     * @param slot Drum pad index (0–15).
     * @param velocity Linear velocity [0.0, 1.0], mapped to MIDI 1–127.
     */
    void triggerSlot (int slot, float velocity) override;

    /**
     * @brief Sets the track's volume.
     *
     * Applies volume to the track's VolumePlugin, affecting all samples
     * played on this track.
     *
     * @param linear01 Linear gain in [0.0, 1.0].
     */
    void setVolume (float linear01) override;

    /**
     * @brief Sets ADSR envelope parameters (currently unused).
     *
     * SamplerPlugin uses per-sample envelope settings rather than a global
     * ADSR, so this is currently a no-op.
     *
     * @param a Attack time.
     * @param d Decay time.
     * @param s Sustain level.
     * @param r Release time.
     */
    void setADSR (float a, float d, float s, float r) override;

    /**
     * @brief Returns the display name for a given drum pad slot.
     *
     * Defaults to pitch names for the mapped note (e.g., "C1", "C#1") until
     * a sample is loaded, after which it becomes the file's base name.
     *
     * @param slot Drum pad index (0–15).
     * @return Display name for the pad.
     */
    [[nodiscard]] juce::String getSlotName (int slot) const override;

    //==============================================================================
    // Public Accessors

    /**
     * @brief Returns the underlying Tracktion SamplerPlugin.
     *
     * Exposes the SamplerPlugin for advanced or custom operations.
     *
     * @return Pointer to SamplerPlugin (may be nullptr if creation failed).
     */
    [[nodiscard]] te::SamplerPlugin* getSampler() const noexcept { return sampler; }

private:
    //==============================================================================
    // Internal Methods

    /**
     * @brief Finds an existing SamplerPlugin on the track or creates a new one.
     *
     * Iterates the track's plugin list, returning the first SamplerPlugin if one
     * is found. If none exists, a new SamplerPlugin is created via the plugin
     * cache and inserted at index 0.
     *
     * @return Pointer to SamplerPlugin or nullptr if creation fails.
     */
    te::SamplerPlugin* findOrCreateSampler();

    /**
     * @brief Finds the sound index mapped to a particular MIDI note.
     *
     * @param note MIDI note number.
     * @return Index of the sound mapped to this note, or -1 if none.
     */
    int findSoundForNote (int note) const;

    /**
     * @brief Returns the length of an audio file in seconds.
     *
     * Uses a JUCE AudioFormatReader to obtain the duration from metadata.
     *
     * @param file Audio file to inspect.
     * @return Length in seconds, or 0.0 if the file cannot be read.
     */
    static double fileLengthSeconds (const juce::File& file);

    //==============================================================================
    // Member Variables

    te::Engine& engine;          ///< Reference to Tracktion Engine (not owned).
    te::AudioTrack& track;       ///< Reference to audio track (not owned).
    te::SamplerPlugin* sampler = nullptr; ///< Underlying sampler plugin (not owned).

    /**
     * @brief Cached display names for 16 drum pads.
     *
     * Initially set to pitch names for the mapped MIDI notes. Updated to the
     * sample file base name when a sample is loaded into the pad.
     */
    juce::Array<juce::String> slotNames
    {
        "C1",  "C#1", "D1",  "D#1",
        "E1",  "F1",  "F#1", "G1",
        "G#1", "A1",  "A#1", "B1",
        "C2",  "C#2", "D2",  "D#2"
    };
};