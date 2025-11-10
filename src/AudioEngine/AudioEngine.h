#pragma once
#include "../MIDIEngine/MIDIEngine.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

class AudioEngine
{
public:
    AudioEngine (te::Edit& editRef, te::Engine& engine);
    ~AudioEngine();

    void play();
    void stop();
    bool isPlaying() const;

    juce::StringArray listOutputDevices() const;
    bool setOutputDeviceByName (const juce::String& deviceName);
    bool setDefaultOutputDevice();
    juce::String getCurrentOutputDeviceName() const;
    juce::AudioDeviceManager& getAudioDeviceManager();
    void initialiseDefaults (double sampleRate = 48000.0, int bufferSize = 512);

    // MIDI Input device management (Tracktion InputDevice system)
    void setupMidiInputDevices(te::Edit& edit);
    void routeMidiToTrack(te::Edit& edit, int trackIndex);
    void clearMidiRouting(te::Edit& edit);
    juce::StringArray listMidiInputDevices() const;
    void logAvailableMidiDevices() const;

private:
    te::Edit& edit;
    std::unique_ptr<MIDIEngine> midiEngine;
    te::Engine& engine;
    juce::AudioDeviceManager& adm() const;
    bool applySetup (const juce::AudioDeviceManager::AudioDeviceSetup& setup);
};
