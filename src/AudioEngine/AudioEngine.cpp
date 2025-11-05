#include "AudioEngine.h"
using namespace juce;

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

AudioEngine::AudioEngine(te::Edit& editRef, te::Engine& engine)
    : edit(editRef), engine(engine)
{
    midiEngine = std::make_unique<MIDIEngine>(edit);
}

AudioEngine::~AudioEngine() = default;

void AudioEngine::play()
{
    auto& transport = edit.getTransport();

    if (transport.looping)
        transport.setPosition(transport.getLoopRange().getStart());

    if (!transport.isPlaying())
        transport.play(false);
}

void AudioEngine::stop() {
    if (&edit)
    {
        edit.getTransport().stop(false, false);
    }
}

bool AudioEngine::isPlaying() const { return edit.getTransport().isPlaying(); }

AudioDeviceManager& AudioEngine::adm() const
{
    return engine.getDeviceManager().deviceManager;
}

void AudioEngine::initialiseDefaults (double sampleRate, int bufferSize)
{
    auto& dm = adm();

    dm.setCurrentAudioDeviceType ("CoreAudio", true); // macOS

    AudioDeviceManager::AudioDeviceSetup setup;
    dm.getAudioDeviceSetup (setup);

    setup.inputDeviceName.clear();
    setup.useDefaultInputChannels  = false;
    setup.useDefaultOutputChannels = true;

    if (setup.sampleRate == 0) setup.sampleRate = sampleRate;
    if (setup.bufferSize  == 0) setup.bufferSize  = bufferSize;

    applySetup (setup);

    // Log available MIDI input devices on startup
    logAvailableMidiDevices();
}

StringArray AudioEngine::listOutputDevices() const
{
    StringArray out;
    auto& dm = const_cast<AudioEngine*>(this)->adm();
    if (auto* type = dm.getCurrentDeviceTypeObject())
        out = type->getDeviceNames (false /* outputs */);
    out.removeEmptyStrings();
    return out;
}

bool AudioEngine::setOutputDeviceByName (const String& deviceName)
{
    auto& dm = adm();
    auto* type = dm.getCurrentDeviceTypeObject();
    if (type == nullptr)
        return false;

    auto outs = type->getDeviceNames (false);
    if (! outs.contains (deviceName))
    {
        Logger::writeToLog ("[Audio] Device not found: " + deviceName);
        return false;
    }

    AudioDeviceManager::AudioDeviceSetup setup;
    dm.getAudioDeviceSetup (setup);

    setup.outputDeviceName          = deviceName;
    setup.inputDeviceName           = {};
    setup.useDefaultInputChannels   = false;
    setup.useDefaultOutputChannels  = true;

    if (setup.sampleRate == 0) setup.sampleRate = 48000.0;
    if (setup.bufferSize  == 0) setup.bufferSize  = 512;

    return applySetup (setup);
}

bool AudioEngine::setDefaultOutputDevice()
{
    auto& dm = adm();

    AudioDeviceManager::AudioDeviceSetup setup;
    dm.getAudioDeviceSetup (setup);

    setup.outputDeviceName.clear();
    setup.useDefaultOutputChannels = true;

    return applySetup (setup);
}

String AudioEngine::getCurrentOutputDeviceName() const
{
    auto& dm = const_cast<AudioEngine*>(this)->adm();
    if (auto* dev = dm.getCurrentAudioDevice())
        return dev->getName();
    return {};
}

AudioDeviceManager& AudioEngine::getAudioDeviceManager()
{
    return adm();
}

bool AudioEngine::applySetup (const AudioDeviceManager::AudioDeviceSetup& newSetup)
{
    auto& dm = adm();
    auto err = dm.setAudioDeviceSetup (newSetup, true);
    if (err.isNotEmpty())
    {
        Logger::writeToLog ("[Audio] setAudioDeviceSetup error: " + err);
        return false;
    }

    Logger::writeToLog ("[Audio] Output now: " + getCurrentOutputDeviceName());

    return true;
}

StringArray AudioEngine::listMidiInputDevices() const
{
    StringArray deviceNames;
    auto devices = juce::MidiInput::getAvailableDevices();

    for (const auto& device : devices)
        deviceNames.add(device.name);

    return deviceNames;
}

void AudioEngine::logAvailableMidiDevices() const
{
    auto devices = juce::MidiInput::getAvailableDevices();

    Logger::writeToLog ("[MIDI] Available MIDI Input Devices:");

    if (devices.isEmpty())
    {
        Logger::writeToLog ("[MIDI]   No MIDI input devices found");
    }
    else
    {
        for (int i = 0; i < devices.size(); ++i)
        {
            const auto& device = devices[i];
            Logger::writeToLog ("[MIDI]   [" + String(i) + "] " + device.name +
                              " (ID: " + device.identifier + ")");
        }
    }
}

bool AudioEngine::setMidiInputDeviceEnabled(int deviceIndex, bool enabled)
{
    auto devices = juce::MidiInput::getAvailableDevices();

    if (deviceIndex < 0 || deviceIndex >= devices.size())
    {
        Logger::writeToLog("[MIDI] Invalid device index: " + String(deviceIndex));
        return false;
    }

    const auto& device = devices[deviceIndex];
    auto& dm = adm();

    dm.setMidiInputDeviceEnabled(device.identifier, enabled);

    Logger::writeToLog("[MIDI] Device " + String(enabled ? "enabled" : "disabled") +
                      ": " + device.name);

    return true;
}

bool AudioEngine::connectMidiInputToCallback(int deviceIndex, juce::MidiInputCallback* callback)
{
    auto devices = juce::MidiInput::getAvailableDevices();

    if (deviceIndex < 0 || deviceIndex >= devices.size())
    {
        Logger::writeToLog("[MIDI] Invalid device index: " + String(deviceIndex));
        return false;
    }

    const auto& device = devices[deviceIndex];
    auto& dm = adm();

    // Enable the device first
    dm.setMidiInputDeviceEnabled(device.identifier, true);

    // Add the callback to receive MIDI messages
    dm.addMidiInputDeviceCallback(device.identifier, callback);

    // Track the currently connected device
    currentMidiInputIdentifier = device.identifier;

    Logger::writeToLog("[MIDI] Connected to device: " + device.name);

    return true;
}

bool AudioEngine::setMidiInputDeviceByName(const juce::String& deviceName, juce::MidiInputCallback* callback)
{
    auto devices = juce::MidiInput::getAvailableDevices();

    // Find device by name
    for (const auto& device : devices)
    {
        if (device.name == deviceName)
        {
            auto& dm = adm();

            // Disconnect previous device if any
            if (currentMidiInputIdentifier.isNotEmpty())
            {
                dm.removeMidiInputDeviceCallback(currentMidiInputIdentifier, callback);
                dm.setMidiInputDeviceEnabled(currentMidiInputIdentifier, false);
            }

            // Enable the new device
            dm.setMidiInputDeviceEnabled(device.identifier, true);

            // Add the callback to receive MIDI messages
            dm.addMidiInputDeviceCallback(device.identifier, callback);

            // Track the currently connected device
            currentMidiInputIdentifier = device.identifier;

            Logger::writeToLog("[MIDI] Connected to device: " + device.name);

            return true;
        }
    }

    Logger::writeToLog("[MIDI] Device not found: " + deviceName);
    return false;
}

juce::String AudioEngine::getCurrentMidiInputDeviceName() const
{
    if (currentMidiInputIdentifier.isEmpty())
        return {};

    auto devices = juce::MidiInput::getAvailableDevices();

    for (const auto& device : devices)
    {
        if (device.identifier == currentMidiInputIdentifier)
            return device.name;
    }

    return {};
}

void AudioEngine::disconnectAllMidiInputs(juce::MidiInputCallback* callback)
{
    if (currentMidiInputIdentifier.isEmpty())
        return;

    auto& dm = adm();

    dm.removeMidiInputDeviceCallback(currentMidiInputIdentifier, callback);
    dm.setMidiInputDeviceEnabled(currentMidiInputIdentifier, false);

    currentMidiInputIdentifier.clear();

    Logger::writeToLog("[MIDI] Disconnected all MIDI input devices");
}


