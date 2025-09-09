#pragma once
#include "DrumSamplerEngine.h"
#include <tracktion_engine/tracktion_engine.h>

// Short alias
namespace te = tracktion_engine;

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
        const int safeSlot = juce::jlimit (0, 15, slot);

        // Tracktion SamplerPlugin stores multiple samples each with a root note/keyrange.
        // We'll add (or replace) a sample that responds to a single MIDI note = padToMidiNote(slot).

        // Try to reuse if a sample already exists for this note:
        te::SamplerPlugin::Sample* existing = nullptr;
        for (auto* s : sampler->getSamples())
            if (s != nullptr && s->getRootNote() == padToMidiNote (safeSlot))
                { existing = s; break; }

        if (existing)
        {
            existing->loadFromFile (file, true /*async ok*/, nullptr);
            existing->setKeyRange (padToMidiNote (safeSlot), padToMidiNote (safeSlot));
        }
        else
        {
            auto* s = sampler->addNewSample();
            if (s != nullptr)
            {
                s->loadFromFile (file, true, nullptr);
                s->setRootNote (padToMidiNote (safeSlot));
                s->setKeyRange (padToMidiNote (safeSlot), padToMidiNote (safeSlot));
            }
        }

        slotNames[safeSlot] = file.getFileNameWithoutExtension();
    }

    void triggerSlot (int slot, float velocity) override
    {
        // Fire a short MIDI note on the track’s input node; Tracktion will deliver it to the plugin.
        const int note = padToMidiNote (slot);
        const int vel  = juce::jlimit (1, 127, (int) juce::roundToInt (velocity * 127.0f));

        // Send immediate note-on/off (very short) into the edit’s MIDI input.
        auto& edit = track.edit;
        auto now   = edit.getTransport().getCurrentPosition();
        te::MidiMessageArray mma;
        mma.addEvent (juce::MidiMessage::noteOn  (1, note, (juce::uint8) vel), 0);
        mma.addEvent (juce::MidiMessage::noteOff (1, note),                    240); // ~5ms later at 48k
        edit.getMidiInputDevices().forEachDevice ([&](te::InputDeviceInstance& dev){ dev.injectMidiMessageArray (mma); });
    }

    void setVolume (float linear01) override
    {
        // Set track volume (safer than per-plugin gain—works for whole drum chain)
        track.getVolumePlugin()->setVolumeDb (juce::Decibels::gainToDecibels (juce::jlimit (0.0f, 1.0f, linear01)));
    }

    void setADSR (float a, float d, float s, float r) override
    {
        if (! sampler) return;
        // Apply to all samples (simple global behavior)
        for (auto* smp : sampler->getSamples())
            if (smp != nullptr)
                smp->setADSR (a, d, s, r);
    }

    juce::String getSlotName (int slot) const override
    {
        return slotNames[juce::jlimit (0, 15, slot)];
    }

    te::SamplerPlugin* getSampler() const { return sampler; }

private:
    te::SamplerPlugin* findOrCreateSampler()
    {
        // Try to find existing Sampler
        for (auto& pi : track.pluginList)
            if (auto* sp = dynamic_cast<te::SamplerPlugin*> (pi.get()))
                return sp;

        // Otherwise, create one
        auto& ec = engine.getPluginManager().pluginTypes;
        te::Plugin::Ptr plugin;

        // Try native Sampler type
        if (auto* t = ec.findType ("Sampler"))
            plugin = t->create (*track.edit);

        if (! plugin)
            return nullptr;

        track.pluginList.insertPlugin (plugin, 0, nullptr);
        return dynamic_cast<te::SamplerPlugin*> (plugin.get());
    }

    te::Engine& engine;
    te::AudioTrack& track;
    te::SamplerPlugin* sampler = nullptr;

    juce::String slotNames[16];
};
