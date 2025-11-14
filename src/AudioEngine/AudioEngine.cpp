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
    if (&edit)
    {
        // First param = false: DON'T discard recordings (keep them!)
        // Second param = false: Don't clear devices
        edit.getTransport().stop(false, false);
        for (auto* plugin : te::getAllPlugins (edit, false))
            if (auto* morph = dynamic_cast<MorphSynthPlugin*>(plugin))
                morph->stopAllNotes();
    }
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

    Logger::writeToLog("[MIDI] Enabled all MIDI input devices via Tracktion InputDevice system");
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

//==============================================================================
// Recording Control

void AudioEngine::toggleRecord(te::Edit& editToRecord)
{
    auto& transport = editToRecord.getTransport();
    bool wasRecording = transport.isRecording();
    bool wasPlaying = transport.isPlaying();

    // Log call stack to see who's calling us
    static int callCount = 0;
    callCount++;
    Logger::writeToLog("[DEBUG] ========== toggleRecord CALLED (call #" + String(callCount) + ") ==========");
    Logger::writeToLog("[DEBUG] Current state: isRecording=" + String(wasRecording ? "TRUE" : "FALSE") +
                       ", isPlaying=" + String(wasPlaying ? "TRUE" : "FALSE"));

    // Check loop range
    if (transport.looping)
    {
        auto loopRange = transport.getLoopRange();
        Logger::writeToLog("[DEBUG] Looping is ENABLED: " +
                         String(loopRange.getStart().inSeconds()) + "s to " +
                         String(loopRange.getEnd().inSeconds()) + "s (" +
                         String(loopRange.getLength().inSeconds()) + "s duration)");
    }
    else
    {
        Logger::writeToLog("[DEBUG] Looping is DISABLED");
    }

    // Check armed tracks and their recording state
    auto tracks = te::getAudioTracks(editToRecord);
    Logger::writeToLog("[DEBUG] Total tracks: " + String(tracks.size()));

    for (int i = 0; i < tracks.size(); ++i)
    {
        auto* track = tracks[i];
        bool trackArmed = false;

        for (auto* instance : editToRecord.getAllInputDevices())
        {
            if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
            {
                if (instance->isRecordingEnabled(track->itemID))
                {
                    trackArmed = true;
                    Logger::writeToLog("[DEBUG] Track " + String(i) + " (" + track->getName() + ") is ARMED for recording");
                    break;
                }
            }
        }

        if (!trackArmed)
            Logger::writeToLog("[DEBUG] Track " + String(i) + " (" + track->getName() + ") is NOT armed");
    }

    // Position at loop start if we're about to start recording and looping is enabled
    if (!wasRecording && transport.looping)
    {
        Logger::writeToLog("[DEBUG] Positioning at loop start before recording");
        transport.setPosition(transport.getLoopRange().getStart());
    }

    // Check playback context allocation before recording
    if (!wasRecording)
    {
        auto* context = transport.getCurrentPlaybackContext();
        Logger::writeToLog("[DEBUG] Playback context before record: " +
                         String(context != nullptr ? "ALLOCATED" : "NULL"));
    }

    // Use Tracktion's battle-tested helper function
    Logger::writeToLog("[DEBUG] Calling EngineHelpers::toggleRecord()...");
    EngineHelpers::toggleRecord(editToRecord);

    // Check state immediately after toggle
    bool nowRecording = transport.isRecording();
    bool nowPlaying = transport.isPlaying();
    Logger::writeToLog("[DEBUG] After toggleRecord: isRecording=" + String(nowRecording ? "TRUE" : "FALSE") +
                       ", isPlaying=" + String(nowPlaying ? "TRUE" : "FALSE"));

    // If we just started recording, check context again
    if (!wasRecording && nowRecording)
    {
        auto* context = transport.getCurrentPlaybackContext();
        Logger::writeToLog("[DEBUG] Playback context after record start: " +
                         String(context != nullptr ? "ALLOCATED" : "NULL"));
    }

    // Log clip count after stopping recording
    if (wasRecording)
    {
        Logger::writeToLog("[DEBUG] Recording was just STOPPED - checking for new clips...");

        // Try to flush state to ensure clips are finalized
        Logger::writeToLog("[DEBUG] Flushing edit state...");
        editToRecord.flushState();

        // IMMEDIATE CHECK
        Logger::writeToLog("[DEBUG] === IMMEDIATE CHECK (right after stop) ===");
        Logger::writeToLog("[DEBUG] Checking ALL tracks in edit for clips:");
        Logger::writeToLog("[DEBUG] Total audio tracks in edit: " + String(tracks.size()));

        for (int i = 0; i < tracks.size(); ++i)
        {
            auto* track = tracks[i];

            // Check all clips on track, not just MIDI clips
            int totalClips = track->getClips().size();
            Logger::writeToLog("[DEBUG] Track " + String(i) + " (" + track->getName() + ") has " +
                             String(totalClips) + " total clips (IMMEDIATE)");

            int clipCount = 0;
            for (auto* clip : track->getClips())
            {
                if (auto* midiClip = dynamic_cast<te::MidiClip*>(clip))
                {
                    clipCount++;
                    auto& sequence = midiClip->getSequence();
                    Logger::writeToLog("[DEBUG]   MIDI Clip " + String(clipCount) + ": " +
                                     "start=" + String(midiClip->getPosition().getStart().inSeconds()) + "s, " +
                                     "length=" + String(midiClip->getPosition().getLength().inSeconds()) + "s, " +
                                     "notes=" + String(sequence.getNumNotes()));
                }
                else
                {
                    Logger::writeToLog("[DEBUG]   Non-MIDI clip found: " + clip->getName() +
                                     " (type: " + clip->getSelectableDescription() + ")");
                }
            }

            if (clipCount == 0)
                Logger::writeToLog("[DEBUG] Track " + String(i) + " has NO MIDI clips (IMMEDIATE)!");
        }

        // DELAYED CHECK - wait for async clip finalization
        Logger::writeToLog("[DEBUG] === Scheduling DELAYED CHECK (500ms later) ===");
        juce::MessageManager::callAsync([&editToRecord]()
        {
            // Wait a bit for Tracktion to finalize clips
            juce::Thread::sleep(500);

            Logger::writeToLog("[DEBUG] === DELAYED CHECK (500ms after stop) ===");
            auto delayedTracks = te::getAudioTracks(editToRecord);
            Logger::writeToLog("[DEBUG] Total audio tracks in edit: " + String(delayedTracks.size()));

            for (int i = 0; i < delayedTracks.size(); ++i)
            {
                auto* track = delayedTracks[i];

                // Check all clips on track, not just MIDI clips
                int totalClips = track->getClips().size();
                Logger::writeToLog("[DEBUG] Track " + String(i) + " (" + track->getName() + ") has " +
                                 String(totalClips) + " total clips (DELAYED)");

                int clipCount = 0;
                for (auto* clip : track->getClips())
                {
                    if (auto* midiClip = dynamic_cast<te::MidiClip*>(clip))
                    {
                        clipCount++;
                        auto& sequence = midiClip->getSequence();
                        Logger::writeToLog("[DEBUG]   MIDI Clip " + String(clipCount) + ": " +
                                         "start=" + String(midiClip->getPosition().getStart().inSeconds()) + "s, " +
                                         "length=" + String(midiClip->getPosition().getLength().inSeconds()) + "s, " +
                                         "notes=" + String(sequence.getNumNotes()));
                    }
                    else
                    {
                        Logger::writeToLog("[DEBUG]   Non-MIDI clip found: " + clip->getName() +
                                         " (type: " + clip->getSelectableDescription() + ")");
                    }
                }

                if (clipCount == 0)
                    Logger::writeToLog("[DEBUG] Track " + String(i) + " has NO MIDI clips (DELAYED)!");
            }
        });
    }
    else
    {
        Logger::writeToLog("[DEBUG] Recording was just STARTED");
    }

    Logger::writeToLog("[DEBUG] ========== toggleRecord END ==========");
}

bool AudioEngine::isRecording() const
{
    // Check if the transport is in recording mode
    return edit.getTransport().isRecording();
}


