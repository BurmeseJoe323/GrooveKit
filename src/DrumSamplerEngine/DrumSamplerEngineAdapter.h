#pragma once
#include "../UI/DrumSamplerView/DrumSamplerEngine.h"
#include <tracktion_engine/tracktion_engine.h>
#include <tracktion_graph/tracktion_graph.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <utility>
namespace te = tracktion::engine;

/**
 * @brief Converts drum pad index (0-15) to MIDI note number (36-51).
 *
 * Maps pad layout to General MIDI drum map range:
 *  - Pad 0 → MIDI note 36 (Kick)
 *  - Pad 15 → MIDI note 51 (Ride cymbal)
 *
 * @param padIndex Pad index (0-15, clamped if out of range)
 * @return MIDI note number (36-51)
 */
static inline int padToMidiNote (int padIndex) { return 36 + juce::jlimit (0, 15, padIndex); }

/**
 * @brief Adapts Tracktion Engine's SamplerPlugin to GrooveKit's DrumSamplerEngine interface.
 *
 * DrumSamplerEngineAdapter bridges the gap between GrooveKit's abstract DrumSamplerEngine
 * interface (used by DrumSamplerView UI) and Tracktion Engine's concrete SamplerPlugin.
 * It manages a 16-pad drum sampler by:
 *  - Creating/finding SamplerPlugin instance on the track
 *  - Loading WAV files into specific MIDI note slots (36-51)
 *  - Triggering samples via live MIDI injection
 *  - Managing per-pad metadata (sample names)
 *
 * Architecture:
 *  - Owned by TrackManager (one adapter per drum track)
 *  - Wraps existing te::AudioTrack and te::SamplerPlugin (not owned)
 *  - Uses MIDI note injection for playback (track.injectLiveMidiMessage)
 *  - Automatically creates SamplerPlugin if track doesn't have one
 *
 * MIDI Note Mapping:
 *  - 16 drum pads map to MIDI notes 36-51 (General MIDI drum map range)
 *  - Each pad triggers a single MIDI note (keyNote == minNote == maxNote)
 *  - Note-off sent automatically after 80ms for one-shot samples
 *
 * Sample Management:
 *  - Uses Tracktion's addSound() for new samples
 *  - Reuses existing sound slots via findSoundForNote() to avoid duplicates
 *  - Queries file length via JUCE AudioFormatReader for proper looping
 *
 * Usage:
 *  - Created by TrackManager::addTrack() for drum tracks
 *  - DrumSamplerView calls loadSampleIntoSlot() to assign WAV files
 *  - DrumPadComponent calls triggerSlot() for playback
 */
class DrumSamplerEngineAdapter : public DrumSamplerEngine
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the adapter for a given track.
     *
     * Finds existing SamplerPlugin or creates new one automatically.
     * Initializes 16 pad names to "Pad 1" through "Pad 16".
     *
     * @param eng Tracktion Engine instance (not owned)
     * @param trk Audio track to attach sampler to (not owned)
     */
    DrumSamplerEngineAdapter (te::Engine& eng, te::AudioTrack& trk)
        : engine (eng), track (trk)
    {
        sampler = findOrCreateSampler();

        for (int i = 0; i < 16; ++i)
            slotNames[i] = "Pad " + juce::String (i + 1);
    }

    //==============================================================================
    // DrumSamplerEngine Overrides

    /**
     * @brief Loads a WAV file into a drum pad slot.
     *
     * Assigns the sample to a specific MIDI note (36-51) in the SamplerPlugin.
     * Reuses existing sound slot if one is already mapped to this note, otherwise
     * creates new sound via addSound(). Updates slotNames array with file name.
     *
     * @param slot Drum pad index (0-15)
     * @param file WAV file to load
     */
    void loadSampleIntoSlot (int slot, const juce::File& file) override
    {
        if (! sampler) return;

        const int pad   = juce::jlimit (0, 15, slot);
        const int note  = padToMidiNote (pad);
        const auto name = file.getFileNameWithoutExtension();

        if (int idx = findSoundForNote (note); idx >= 0)
        {
            sampler->setSoundMedia  (idx, file.getFullPathName());
            sampler->setSoundParams (idx, note, note, note);
            sampler->setSoundName   (idx, name);
            DBG ("[Sampler] Reused sound idx=" << idx << " for note " << note << " -> " << file.getFileName());
        }
        else
        {
            const double len   = fileLengthSeconds (file);
            const int    before = sampler->getNumSounds();

            sampler->addSound (file.getFullPathName(),
                               name,
                               0.0,
                               len > 0.0 ? len : 1.0,
                               0.0f);

            const int index = (sampler->getNumSounds() > before ? before
                                                                : sampler->getNumSounds() - 1);

            if (index >= 0)
            {
                sampler->setSoundParams (index, note, note, note);
                sampler->setSoundName   (index, name);
                DBG ("[Sampler] Added new sound idx=" << index << " for note " << note << " -> " << file.getFileName());
            }
            else
            {
                DBG ("[Sampler][WARN] addSound didn't create a sound (count=" << sampler->getNumSounds() << ")");
            }
        }

        slotNames.set (pad, name);
    }

    /**
     * @brief Triggers a drum pad with specified velocity.
     *
     * Injects MIDI note-on message via track.injectLiveMidiMessage() for immediate
     * playback. Automatically sends note-off after 80ms for one-shot behavior.
     * Requires valid sound mapped to pad's MIDI note.
     *
     * @param slot Drum pad index (0-15)
     * @param velocity Linear velocity (0.0-1.0, converted to MIDI 1-127)
     */
    void triggerSlot (int slot, float velocity) override
    {
        if (! sampler) return;

        const int note = padToMidiNote (slot);
        const juce::uint8 vel = (juce::uint8) juce::jlimit (1, 127,
                                   (int) juce::roundToInt (velocity * 127.0f));

        const int mappedIdx = findSoundForNote (note);
        if (mappedIdx < 0)
        {
            DBG ("[Sampler] No sound mapped for note " << note << " (slot " << slot << ")");
            return;
        }

        track.edit.getTransport().ensureContextAllocated();

        const te::MPESourceID source ((juce::uint8) 1);

        juce::MidiMessage on  = juce::MidiMessage::noteOn  (1, note, vel);
        track.injectLiveMidiMessage (te::MidiMessageWithSource (on, source));

        juce::MidiMessage off = juce::MidiMessage::noteOff (1, note);
        juce::Timer::callAfterDelay (80, [this, off, source]
        {
            track.injectLiveMidiMessage (te::MidiMessageWithSource (off, source));
        });
    }

    /**
     * @brief Sets the track volume.
     *
     * Applies volume to track's VolumePlugin, affecting all samples on this track.
     *
     * @param linear01 Linear volume (0.0-1.0, converted to dB)
     */
    void setVolume (float linear01) override
    {
        track.getVolumePlugin()->setVolumeDb (juce::Decibels::gainToDecibels (juce::jlimit (0.0f, 1.0f, linear01)));
    }

    /**
     * @brief Sets ADSR envelope parameters (currently unimplemented).
     *
     * Reserved for future envelope control. SamplerPlugin uses per-sample
     * envelope settings, not global ADSR.
     *
     * @param a Attack time
     * @param d Decay time
     * @param s Sustain level
     * @param r Release time
     */
    void setADSR (float a, float d, float s, float r) override
    {
        juce::ignoreUnused (a, d, s, r);
    }

    /**
     * @brief Returns the display name for a drum pad slot.
     *
     * Returns sample file name if loaded, otherwise "Pad N".
     *
     * @param slot Drum pad index (0-15)
     * @return Display name for the slot
     */
    [[nodiscard]] juce::String getSlotName (int slot) const override
    {
        return slotNames[juce::jlimit (0, 15, slot)];
    }

    //==============================================================================
    // Public Accessors

    /**
     * @brief Returns the underlying Tracktion SamplerPlugin.
     *
     * Allows direct access to SamplerPlugin for advanced operations.
     *
     * @return Pointer to SamplerPlugin (may be nullptr if creation failed)
     */
    [[nodiscard]] te::SamplerPlugin* getSampler() const { return sampler; }

private:
    //==============================================================================
    // Internal Methods

    /**
     * @brief Finds existing SamplerPlugin on track or creates new one.
     *
     * Searches track's plugin list for SamplerPlugin. If not found, creates
     * new instance via plugin cache and inserts at index 0.
     *
     * @return Pointer to SamplerPlugin, or nullptr if creation failed
     */
    te::SamplerPlugin* findOrCreateSampler()
    {
        for (auto* p : track.pluginList.getPlugins())
            if (auto* sp = dynamic_cast<te::SamplerPlugin*>(p))
                return sp;

        te::Plugin::Ptr plugin = track.edit.getPluginCache()
                                     .createNewPlugin(te::SamplerPlugin::xmlTypeName, {});
        if (!plugin)
            return nullptr;

        track.pluginList.insertPlugin(plugin, 0, nullptr);
        return dynamic_cast<te::SamplerPlugin*>(plugin.get());
    }

    /**
     * @brief Finds sound slot index for a given MIDI note.
     *
     * Searches SamplerPlugin's sound list for a sound mapped to the specified
     * MIDI note. Used to check if a pad already has a sample loaded.
     *
     * @param note MIDI note number (36-51 for drum pads)
     * @return Sound index, or -1 if no sound mapped to this note
     */
    int findSoundForNote (int note) const
    {
        if (!sampler) return -1;
        const int n = sampler->getNumSounds();
        for (int i = 0; i < n; ++i)
            if (sampler->getKeyNote (i) == note)
                return i;
        return -1;
    }

    /**
     * @brief Calculates the length of an audio file in seconds.
     *
     * Uses JUCE AudioFormatManager to read WAV file metadata and calculate
     * duration. Required for SamplerPlugin's addSound() method.
     *
     * @param f Audio file to measure
     * @return Length in seconds, or 0.0 if file cannot be read
     */
    static double fileLengthSeconds (const juce::File& f)
    {
        juce::AudioFormatManager fm; fm.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> r (fm.createReaderFor (f));
        if (!r) return 0.0;
        return static_cast<double> (r->lengthInSamples) / r->sampleRate;
    }

    //==============================================================================
    // Member Variables

    te::Engine& engine; ///< Reference to Tracktion Engine (not owned)
    te::AudioTrack& track; ///< Reference to audio track (not owned)
    te::SamplerPlugin* sampler = nullptr; ///< Pointer to track's SamplerPlugin (not owned)

    juce::Array<juce::String> slotNames = {"C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1", "C2", "C#2", "D2", "D#2" }; ///< Cached display names for 16 drum pads
};
