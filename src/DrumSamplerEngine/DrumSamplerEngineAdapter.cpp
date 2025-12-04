#include "DrumSamplerEngineAdapter.h"

//==============================================================================
// Construction / Destruction

DrumSamplerEngineAdapter::DrumSamplerEngineAdapter (te::Engine& eng,
                                                    te::AudioTrack& trk)
    : engine (eng),
      track (trk)
{
    sampler = findOrCreateSampler();

    // Initialize slot names from existing sampler sounds (for loading saved projects)
    if (sampler != nullptr)
    {
        for (int pad = 0; pad < 16; ++pad)
        {
            const int note = padToMidiNote(pad);
            const int soundIdx = findSoundForNote(note);

            if (soundIdx >= 0)
            {
                // Sample exists for this pad - use its name
                juce::String sampleName = sampler->getSoundName(soundIdx);
                if (sampleName.isNotEmpty())
                    slotNames.set(pad, sampleName);
            }
        }
    }
}

//==============================================================================
// DrumSamplerEngine Overrides

void DrumSamplerEngineAdapter::loadSampleIntoSlot (int slot,
                                                   const juce::File& file)
{
    if (sampler == nullptr)
        return;

    const int pad   = juce::jlimit (0, 15, slot);
    const int note  = padToMidiNote (pad);
    const auto name = file.getFileNameWithoutExtension();

    if (int idx = findSoundForNote (note); idx >= 0)
    {
        // Reuse existing sound slot mapped to this note
        sampler->setSoundMedia  (idx, file.getFullPathName());
        sampler->setSoundParams (idx, note, note, note);
        sampler->setSoundName   (idx, name);
    }
    else
    {
        // Create a new sound slot
        const double len    = fileLengthSeconds (file);
        const int    before = sampler->getNumSounds();

        sampler->addSound (file.getFullPathName(),
                           name,
                           0.0,
                           (len > 0.0 ? len : 1.0),
                           0.0f);

        const int numSounds = sampler->getNumSounds();
        const int index     = (numSounds > before ? before : numSounds - 1);

        if (index >= 0)
        {
            sampler->setSoundParams (index, note, note, note);
            sampler->setSoundName   (index, name);
        }
        // If index < 0, we silently fail here – add logging later if needed.
    }

    slotNames.set (pad, name);
}

void DrumSamplerEngineAdapter::triggerSlot (int slot,
                                            float velocity)
{
    if (sampler == nullptr)
        return;

    const int note = padToMidiNote (slot);

    const juce::uint8 vel = (juce::uint8) juce::jlimit (
        1, 127,
        (int) juce::roundToInt (velocity * 127.0f));

    const int mappedIdx = findSoundForNote (note);
    if (mappedIdx < 0)
        return; // No sound mapped for this note, nothing to play.

    // Ensure the transport has a context before injecting MIDI
    track.edit.getTransport().ensureContextAllocated();

    const te::MPESourceID source ((juce::uint8) 1);

    // Note-on
    juce::MidiMessage on = juce::MidiMessage::noteOn (1, note, vel);
    track.injectLiveMidiMessage (te::MidiMessageWithSource (on, source));

    // Note-off after a short delay (one-shot behavior)
    juce::MidiMessage off = juce::MidiMessage::noteOff (1, note);
    juce::Timer::callAfterDelay (80, [this, off, source]
    {
        track.injectLiveMidiMessage (te::MidiMessageWithSource (off, source));
    });
}

void DrumSamplerEngineAdapter::setVolume (float linear01)
{
    const float clamped = juce::jlimit (0.0f, 1.0f, linear01);
    if (auto* vol = track.getVolumePlugin())
        vol->setVolumeDb (juce::Decibels::gainToDecibels (clamped));
}

void DrumSamplerEngineAdapter::setADSR (float a,
                                        float d,
                                        float s,
                                        float r)
{
    juce::ignoreUnused (a, d, s, r);
    // Reserved for future envelope control if needed.
}

juce::String DrumSamplerEngineAdapter::getSlotName (int slot) const
{
    return slotNames[juce::jlimit (0, 15, slot)];
}

//==============================================================================
// Internal Methods

te::SamplerPlugin* DrumSamplerEngineAdapter::findOrCreateSampler()
{
    // Try to find an existing SamplerPlugin on this track
    for (auto* p : track.pluginList.getPlugins())
        if (auto* sp = dynamic_cast<te::SamplerPlugin*> (p))
            return sp;

    // None found, create a new one
    te::Plugin::Ptr plugin =
        track.edit.getPluginCache().createNewPlugin (te::SamplerPlugin::xmlTypeName, {});

    if (plugin == nullptr)
        return nullptr;

    track.pluginList.insertPlugin (plugin, 0, nullptr);
    return dynamic_cast<te::SamplerPlugin*> (plugin.get());
}

int DrumSamplerEngineAdapter::findSoundForNote (int note) const
{
    if (sampler == nullptr)
        return -1;

    const int numSounds = sampler->getNumSounds();
    for (int i = 0; i < numSounds; ++i)
        if (sampler->getKeyNote (i) == note)
            return i;

    return -1;
}

double DrumSamplerEngineAdapter::fileLengthSeconds (const juce::File& file)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (file));
    if (reader == nullptr)
        return 0.0;

    return static_cast<double> (reader->lengthInSamples) / reader->sampleRate;
}