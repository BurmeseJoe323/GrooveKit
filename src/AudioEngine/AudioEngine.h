#pragma once
#include "../MIDIEngine/MIDIEngine.h"
#include "EngineHelpers.h"
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

    // Audio configuration (Written by Claude Code)
    juce::Array<int> getAvailableBufferSizes() const;
    juce::Array<double> getAvailableSampleRates() const;
    int getCurrentBufferSize() const;
    double getCurrentSampleRate() const;
    bool setBufferSize (int bufferSize);
    bool setSampleRate (double sampleRate);

    // MIDI Input device management (Tracktion InputDevice system)
    void setupMidiInputDevices(te::Edit& edit);
    void routeMidiToTrack(te::Edit& edit, int trackIndex);
    juce::StringArray listMidiInputDevices() const;
    void logAvailableMidiDevices() const;

    // MIDI device enable/disable (Written by Claude Code)
    juce::StringArray getEnabledMidiDevices() const;
    bool isMidiDeviceEnabled (const juce::String& deviceName) const;
    void setMidiDeviceEnabled (const juce::String& deviceName, bool enabled);

    // Recording control (simplified - uses EngineHelpers)
    void toggleRecord(te::Edit& edit);
    bool isRecording() const;

private:
    te::Edit& edit;
    std::unique_ptr<MIDIEngine> midiEngine;
    te::Engine& engine;
    juce::AudioDeviceManager& adm() const;
    bool applySetup (const juce::AudioDeviceManager::AudioDeviceSetup& setup);
};
