#include "AudioEngine.h"
#include "../UI/Plugins/Synthesizer/MorphSynthPlugin.h"
using namespace juce;

namespace te = tracktion::engine;
namespace t = tracktion;
using namespace std::literals;
using namespace t::literals;

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
        // First param = false: Keep any recordings in progress (don't discard) (Written by Claude Code)
        edit.getTransport().stop(false, false);
        for (auto* plugin : te::getAllPlugins (edit, false))
            if (auto* morph = dynamic_cast<MorphSynthPlugin*>(plugin))
                morph->stopAllNotes();
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

    Logger::writeToLog("[MIDI] Enabled all MIDI input devices via Tracktion InputDevice system");
}

void AudioEngine::routeMidiToTrack(te::Edit& editToRoute, int trackIndex)
{
    auto tracks = te::getAudioTracks(editToRoute);
    if (trackIndex < 0 || trackIndex >= tracks.size())
    {
        Logger::writeToLog("[MIDI] Invalid track index for routing: " + String(trackIndex));
        return;
    }

    auto* track = tracks[trackIndex];

    // Route all MIDI input devices to this track and pre-arm for recording
    // Following Tracktion's MidiRecordingDemo pattern: set target + enable recording at arm time
    for (auto* instance : editToRoute.getAllInputDevices())
    {
        if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
        {
            // Set target track with undo manager for proper state tracking
            [[maybe_unused]] auto res = instance->setTarget(track->itemID, true, &editToRoute.getUndoManager(), 0);

            // Pre-arm track for recording (following Tracktion demo pattern)
            // This prepares the track to capture MIDI when transport.record() is called
            instance->setRecordingEnabled(track->itemID, true);
        }
    }

    // Restart playback to rebuild audio graph with new routing configuration
    editToRoute.restartPlayback();

    Logger::writeToLog("[MIDI] Routed and pre-armed all MIDI inputs to track " + String(trackIndex));
}

void AudioEngine::toggleRecord(te::Edit& editToRecord)
{
    auto& transport = editToRecord.getTransport();
    bool wasRecording = transport.isRecording();

    Logger::writeToLog("[MIDI Recording] Toggle record - currently recording: " + String(wasRecording ? "yes" : "no"));

    // Position at loop start if we're about to start recording and looping is enabled
    if (!wasRecording && transport.looping)
    {
        Logger::writeToLog("[MIDI Recording] Positioning at loop start before recording");
        transport.setPosition(transport.getLoopRange().getStart());
    }

    // Use Tracktion's battle-tested helper function
    EngineHelpers::toggleRecord(editToRecord);

    // Log clip count after stopping recording
    if (wasRecording)
    {
        Logger::writeToLog("[MIDI Recording] Recording stopped, checking for clips...");
        auto tracks = te::getAudioTracks(editToRecord);
        for (int i = 0; i < tracks.size(); ++i)
        {
            auto* track = tracks[i];
            int clipCount = 0;
            for (auto* clip : track->getClips())
            {
                if (auto* midiClip = dynamic_cast<te::MidiClip*>(clip))
                    clipCount++;
            }
            Logger::writeToLog("[MIDI Recording] Track " + String(i) + " (" + track->getName() + ") has " + String(clipCount) + " MIDI clips");
        }
    }
    else
    {
        Logger::writeToLog("[MIDI Recording] Recording started");
    }
}

bool AudioEngine::isRecording() const
{
    // Check if the transport is in recording mode
    return edit.getTransport().isRecording();
}


