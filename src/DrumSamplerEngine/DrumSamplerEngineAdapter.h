#pragma once
#include "../UI/DrumSamplerView/DrumSamplerEngine.h"
#include <tracktion_engine/tracktion_engine.h>
#include <tracktion_graph/tracktion_graph.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <utility>
namespace te = tracktion;

static inline int padToMidiNote (int padIndex) { return 36 + juce::jlimit (0, 15, padIndex); }

class DrumSamplerEngineAdapter : public DrumSamplerEngine
{
public:
    DrumSamplerEngineAdapter (te::Engine& eng, te::AudioTrack& trk)
        : engine (eng), track (trk)
    {
        sampler = findOrCreateSampler();
    }

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

        slotNames[pad] = name;
    }

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

    void setVolume (float linear01) override
    {
        track.getVolumePlugin()->setVolumeDb (juce::Decibels::gainToDecibels (juce::jlimit (0.0f, 1.0f, linear01)));
    }

    void setADSR (float a, float d, float s, float r) override
    {
        juce::ignoreUnused (a, d, s, r);
    }
    [[nodiscard]] juce::String getSlotName (int slot) const override
    {
        return slotNames[juce::jlimit (0, 15, slot)];
    }

    [[nodiscard]] te::SamplerPlugin* getSampler() const { return sampler; }

private:
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
        return static_cast<double> (r->lengthInSamples) / r->sampleRate;
    }

    te::Engine& engine;
    te::AudioTrack& track;
    te::SamplerPlugin* sampler = nullptr;

    juce::Array<juce::String> slotNames = {"C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1", "C2", "C#2", "D2", "D#2" };
};
