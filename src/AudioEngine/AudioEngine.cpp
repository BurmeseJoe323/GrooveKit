#include "AudioEngine.h"
#include "../UI/Plugins/Synthesizer/MorphSynthPlugin.h"
using namespace juce;

namespace te = tracktion::engine;
namespace t = tracktion;
using namespace std::literals;
using namespace t::literals;

//==============================================================================
// Construction / Destruction

AudioEngine::AudioEngine(te::Edit& editRef, te::Engine& engine)
    : edit(editRef), engine(engine)
{
    midiEngine = std::make_unique<MIDIEngine>(edit);
}

AudioEngine::~AudioEngine() = default;

//==============================================================================
// Transport Control

void AudioEngine::play()
{
    auto& transport = edit.getTransport();

    if (transport.looping)
        transport.setPosition(transport.getLoopRange().getStart());

    if (!transport.isPlaying())
        transport.play(false);
}

void AudioEngine::stop() {
    // First param = false: DON'T discard recordings (keep them!)
    // Second param = false: Don't clear devices
    edit.getTransport().stop(false, false);
    for (auto* plugin : te::getAllPlugins (edit, false))
        if (auto* morph = dynamic_cast<MorphSynthPlugin*>(plugin))
            morph->stopAllNotes();
}

bool AudioEngine::isPlaying() const { return edit.getTransport().isPlaying(); }

//==============================================================================
// Audio Device Management

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

Array<int> AudioEngine::getAvailableBufferSizes() const
{
    auto& dm = const_cast<AudioEngine*>(this)->adm();
    if (auto* device = dm.getCurrentAudioDevice())
        return device->getAvailableBufferSizes();
    return {};
}

Array<double> AudioEngine::getAvailableSampleRates() const
{
    auto& dm = const_cast<AudioEngine*>(this)->adm();
    if (auto* device = dm.getCurrentAudioDevice())
        return device->getAvailableSampleRates();
    return {};
}

int AudioEngine::getCurrentBufferSize() const
{
    auto& dm = const_cast<AudioEngine*>(this)->adm();
    AudioDeviceManager::AudioDeviceSetup setup;
    dm.getAudioDeviceSetup (setup);
    return setup.bufferSize;
}

double AudioEngine::getCurrentSampleRate() const
{
    auto& dm = const_cast<AudioEngine*>(this)->adm();
    AudioDeviceManager::AudioDeviceSetup setup;
    dm.getAudioDeviceSetup (setup);
    return setup.sampleRate;
}

bool AudioEngine::setBufferSize (int bufferSize)
{
    auto& dm = adm();
    AudioDeviceManager::AudioDeviceSetup setup;
    dm.getAudioDeviceSetup (setup);

    setup.bufferSize = bufferSize;

    return applySetup (setup);
}

bool AudioEngine::setSampleRate (double sampleRate)
{
    auto& dm = adm();
    AudioDeviceManager::AudioDeviceSetup setup;
    dm.getAudioDeviceSetup (setup);

    setup.sampleRate = sampleRate;

    return applySetup (setup);
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

//==============================================================================
// MIDI Input Device Management

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

void AudioEngine::setupMidiInputDevices(te::Edit& editToSetup)
{
    // Enable all MIDI input devices in Tracktion's device manager
    for (const auto& midiIn : engine.getDeviceManager().getMidiInDevices())
    {
        midiIn->setEnabled(true);
        // Use 'automatic' mode: monitor when stopped, don't monitor during playback/recording
        midiIn->setMonitorMode(te::InputDevice::MonitorMode::automatic);
    }

    // Ensure transport context is allocated for recording
    editToSetup.getTransport().ensureContextAllocated();

    // Attach MIDI event logger to all physical MIDI inputs for debugging
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (const auto& deviceInfo : midiInputs)
    {
        // Note: We can't directly attach our logger to JUCE's MidiInput from here
        // because Tracktion Engine manages the MIDI inputs internally.
        // Instead, we'll add logging at the Tracktion InputDevice level.
    }

    Logger::writeToLog("[MIDI] Enabled all MIDI input devices via Tracktion InputDevice system");
}

void AudioEngine::setMidiEventLoggingEnabled(bool enable)
{
    midiEventLogger.setEnabled(enable);

    if (enable)
    {
        Logger::writeToLog("[MIDI EVENT LOGGING] ========================================");
        Logger::writeToLog("[MIDI EVENT LOGGING] ENABLED - will log InputDevice state");
        Logger::writeToLog("[MIDI EVENT LOGGING] ========================================");

        // Log current state of all MIDI input device instances
        Logger::writeToLog("[MIDI EVENT LOGGING] Current MIDI Input Device States:");
        for (auto* instance : edit.getAllInputDevices())
        {
            auto& device = instance->getInputDevice();

            if (device.getDeviceType() != te::InputDevice::physicalMidiDevice)
                continue;

            Logger::writeToLog("[MIDI EVENT LOGGING]   Device: " + device.getName());
            Logger::writeToLog("[MIDI EVENT LOGGING]     Enabled: " +
                             String(device.isEnabled() ? "YES" : "NO"));

            auto monitorMode = device.getMonitorMode();
            String modeName = (monitorMode == te::InputDevice::MonitorMode::on) ? "ON" :
                            (monitorMode == te::InputDevice::MonitorMode::off) ? "OFF" : "AUTOMATIC";
            Logger::writeToLog("[MIDI EVENT LOGGING]     Monitor Mode: " + modeName);

            // Check which tracks this device instance is targeting
            auto targetIDs = instance->getTargets();
            if (targetIDs.size() > 0)
            {
                Logger::writeToLog("[MIDI EVENT LOGGING]     Targeting " +
                                 String(targetIDs.size()) + " track(s):");

                // Find track names
                auto tracks = te::getAudioTracks(edit);
                for (const auto& targetID : targetIDs)
                {
                    for (auto* track : tracks)
                    {
                        if (track->itemID == targetID)
                        {
                            Logger::writeToLog("[MIDI EVENT LOGGING]       - Track: " + track->getName() +
                                             " (ID: " + targetID.toString() + ")");
                            Logger::writeToLog("[MIDI EVENT LOGGING]         Recording Enabled: " +
                                             String(instance->isRecordingEnabled(targetID) ? "YES" : "NO"));
                        }
                    }
                }
            }
        }
    }
    else
    {
        Logger::writeToLog("[MIDI EVENT LOGGING] DISABLED");
    }
}

void AudioEngine::routeMidiToTrack(te::Edit& editToRoute, int trackIndex)
{
    Logger::writeToLog("[DEBUG] ========== routeMidiToTrack CALLED ==========");
    Logger::writeToLog("[DEBUG] Target track index: " + String(trackIndex));

    auto tracks = te::getAudioTracks(editToRoute);
    if (trackIndex < 0 || trackIndex >= tracks.size())
    {
        Logger::writeToLog("[MIDI] Invalid track index for routing: " + String(trackIndex));
        return;
    }

    auto* track = tracks[trackIndex];
    Logger::writeToLog("[DEBUG] Target track: " + track->getName() + " (itemID: " + track->itemID.toString() + ")");

    // Route all MIDI input devices to this track and pre-arm for recording
    // Following Tracktion's MidiRecordingDemo pattern: set target + enable recording at arm time
    int deviceCount = 0;
    for (auto* instance : editToRoute.getAllInputDevices())
    {
        if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
        {
            deviceCount++;
            auto& device = instance->getInputDevice();
            Logger::writeToLog("[DEBUG] Processing MIDI device: " + device.getName());

            // Set target track with undo manager for proper state tracking
            auto res = instance->setTarget(track->itemID, true, &editToRoute.getUndoManager(), 0);
            Logger::writeToLog("[DEBUG]   setTarget() returned: " + String(res ? "TRUE" : "FALSE"));

            // Pre-arm track for recording (following Tracktion demo pattern)
            // This prepares the track to capture MIDI when transport.record() is called
            instance->setRecordingEnabled(track->itemID, true);

            // Verify it was actually set
            bool isEnabled = instance->isRecordingEnabled(track->itemID);
            Logger::writeToLog("[DEBUG]   setRecordingEnabled() verification: " +
                             String(isEnabled ? "ENABLED" : "NOT ENABLED"));
        }
    }

    Logger::writeToLog("[DEBUG] Processed " + String(deviceCount) + " MIDI input devices");
    Logger::writeToLog("[DEBUG] ========== routeMidiToTrack END ==========");

    // NOTE: We do NOT call restartPlayback() here (unlike previous implementation).
    // Tracktion's MidiRecordingDemo only calls restartPlayback() during initial setup,
    // not when changing routing. Calling it on every arm was causing audio glitches
    // and the "track created after launch" bug.
}


