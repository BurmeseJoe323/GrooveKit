#pragma once
#include "../UI/DrumSamplerView/DrumSamplerEngine.h"
#include <tracktion_engine/tracktion_engine.h>
#include <tracktion_graph/tracktion_graph.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <utility>
namespace te = tracktion;

// Map 16 pads to GM drum range: 36..51
static inline int padToMidiNote (int padIndex) { return 36 + juce::jlimit (0, 15, padIndex); }

class DrumSamplerEngineAdapter : public DrumSamplerEngine
{
public:
    DrumSamplerEngineAdapter (te::Engine& eng, te::AudioTrack& trk)
        : engine (eng), track (trk)
    {
        // Find or create a Sampler plugin on this track.
        sampler = findOrCreateSampler();
        // Pre-name slots
        for (int i = 0; i < 16; ++i)
            slotNames[i] = "Pad " + juce::String (i + 1);
    }

    // -------- DrumSamplerEngine impl
    void loadSampleIntoSlot (int slot, const juce::File& file) override
    {
        if (! sampler) return;

        const int pad   = juce::jlimit (0, 15, slot);
        const int note  = padToMidiNote (pad);
        const auto name = file.getFileNameWithoutExtension();

        // If a sound already targets this note, just retarget its media + params
        if (int idx = findSoundForNote (note); idx >= 0)
        {
            sampler->setSoundMedia  (idx, file.getFullPathName());
            sampler->setSoundParams (idx, note, note, note);     // root/min/max = this pad
            sampler->setSoundName   (idx, name);
            DBG ("[Sampler] Reused sound idx=" << idx << " for note " << note << " -> " << file.getFileName());
        }
        else
        {
            // Add a new sound and then set its note mapping
            const double len   = fileLengthSeconds (file);       // seconds
            const int    before = sampler->getNumSounds();

            sampler->addSound (file.getFullPathName(),           // path or item ID
                               name,
                               0.0,                               // start time
                               len > 0.0 ? len : 1.0,             // length (fallback to 1s)
                               0.0f);                              // gain dB

            // In TE, addSound appends; the new index is 'before'
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

        slotNames[pad] = name;
    }

    void triggerSlot (int slot, float velocity) override
    {
        if (! sampler) return;

        const int note = padToMidiNote (slot);
        const juce::uint8 vel = (juce::uint8) juce::jlimit (1, 127,
                                   (int) juce::roundToInt (velocity * 127.0f));

        // Sanity: confirm mapping exists
        const int mappedIdx = findSoundForNote (note);
        if (mappedIdx < 0)
        {
            DBG ("[Sampler] No sound mapped for note " << note << " (slot " << slot << ")");
            return;
        }

        // Make sure the audio graph is alive
        track.edit.getTransport().ensureContextAllocated();

        // Your TE wants an MPESourceID (constructor form works on your build)
        const te::MPESourceID source ((juce::uint8) 1);

        // Note-on now…
        juce::MidiMessage on  = juce::MidiMessage::noteOn  (1, note, vel);
        track.injectLiveMidiMessage (te::MidiMessageWithSource (on, source));

        // …and note-off a hair later so it actually sounds
        juce::MidiMessage off = juce::MidiMessage::noteOff (1, note);
        juce::Timer::callAfterDelay (80, [this, off, source]
        {
            track.injectLiveMidiMessage (te::MidiMessageWithSource (off, source));
        });
    }






    void setVolume (float linear01) override
    {
        // Set track volume (safer than per-plugin gain—works for whole drum chain)
        track.getVolumePlugin()->setVolumeDb (juce::Decibels::gainToDecibels (juce::jlimit (0.0f, 1.0f, linear01)));
    }

    void setADSR (float a, float d, float s, float r) override
    {
        juce::ignoreUnused (a, d, s, r);
    }
    juce::String getSlotName (int slot) const override
    {
        return slotNames[juce::jlimit (0, 15, slot)];
    }

    te::SamplerPlugin* getSampler() const { return sampler; }

private:
    te::SamplerPlugin* findOrCreateSampler()
    {
        // 1) Look for an existing Sampler on this track
        // In your TE build, getPlugins() returns a container of raw Plugin*
        for (auto* p : track.pluginList.getPlugins())
            if (auto* sp = dynamic_cast<te::SamplerPlugin*>(p))
                return sp;

        // 2) Otherwise create one via the Edit's PluginCache
        // (this is the same creation path you already use for FourOsc elsewhere)
        te::Plugin::Ptr plugin = track.edit.getPluginCache()
                                     .createNewPlugin(te::SamplerPlugin::xmlTypeName, {});
        if (!plugin)
            return nullptr;

        track.pluginList.insertPlugin(plugin, 0, nullptr);
        return dynamic_cast<te::SamplerPlugin*>(plugin.get());
    }


    int findSoundForNote (int note) const
    {
        if (!sampler) return -1;
        const int n = sampler->getNumSounds();
        for (int i = 0; i < n; ++i)
            if (sampler->getKeyNote (i) == note)
                return i;
        return -1;
    }

    static double fileLengthSeconds (const juce::File& f)
    {
        juce::AudioFormatManager fm; fm.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> r (fm.createReaderFor (f));
        if (!r) return 0.0;
        return r->lengthInSamples / r->sampleRate;
    }

    te::Engine& engine;
    te::AudioTrack& track;
    te::SamplerPlugin* sampler = nullptr;

    juce::String slotNames[16];
};
