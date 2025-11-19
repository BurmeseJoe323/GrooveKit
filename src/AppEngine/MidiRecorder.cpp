#include "MidiRecorder.h"

using namespace juce;
namespace te = tracktion::engine;
namespace t = tracktion;

//==============================================================================
// Construction / Destruction

MidiRecorder::MidiRecorder(te::Engine& eng)
    : engine(eng)
{
}

MidiRecorder::~MidiRecorder()
{
    if (recording && currentEdit)
        stopRecording(*currentEdit);

    detachFromAllSources();
}

//==============================================================================
// Recording Control

void MidiRecorder::startRecording(te::Edit& edit, int trackIndex,
                                  juce::MidiKeyboardState* qwertyKeyboardState)
{
    if (recording)
    {
        Logger::writeToLog("[MidiRecorder] Already recording, stopping previous recording first");
        stopRecording(edit);
    }

    Logger::writeToLog("[MidiRecorder] ========== START RECORDING ==========");
    Logger::writeToLog("[MidiRecorder] Target track index: " + String(trackIndex));

    currentEdit = &edit;
    targetTrackIndex = trackIndex;

    // Clear previous recording buffer
    {
        const ScopedLock sl(recordingLock);
        recordedSequence.clear();
    }

    // Attach to hardware MIDI devices
    attachToHardwareMidiDevices(edit);

    // Attach to QWERTY keyboard if provided
    if (qwertyKeyboardState != nullptr)
    {
        qwertyKeyboardState->addListener(this);
        attachedSources.push_back(qwertyKeyboardState);
        Logger::writeToLog("[MidiRecorder] Attached to QWERTY keyboard state");
    }

    // Record start time and position
    auto& transport = edit.getTransport();
    recordingStartTime = Time::getMillisecondCounterHiRes() / 1000.0;
    recordingStartPosition = transport.getPosition();

    Logger::writeToLog("[MidiRecorder] Recording start position: " +
                      String(recordingStartPosition.inSeconds()) + " seconds");

    // Start transport if not already playing
    if (!transport.isPlaying())
    {
        if (transport.looping)
            transport.setPosition(transport.getLoopRange().getStart());

        transport.play(false);
        Logger::writeToLog("[MidiRecorder] Started transport playback");
    }

    recording = true;
    Logger::writeToLog("[MidiRecorder] Recording ACTIVE - ready to capture MIDI");
}

bool MidiRecorder::stopRecording(te::Edit& edit)
{
    if (!recording)
    {
        Logger::writeToLog("[MidiRecorder] Not currently recording");
        return false;
    }

    Logger::writeToLog("[MidiRecorder] ========== STOP RECORDING ==========");

    recording = false;

    // Detach from all sources
    detachFromAllSources();

    // Check how many events we recorded
    int noteCount = 0;
    {
        const ScopedLock sl(recordingLock);
        noteCount = recordedSequence.getNumEvents();
    }

    Logger::writeToLog("[MidiRecorder] Captured " + String(noteCount) + " MIDI events");

    if (noteCount == 0)
    {
        Logger::writeToLog("[MidiRecorder] No MIDI events recorded - skipping clip creation");
        return false;
    }

    // Create clip from recorded data
    bool success = createClipFromRecording(edit);

    if (success)
        Logger::writeToLog("[MidiRecorder] Successfully created MIDI clip from recording");
    else
        Logger::writeToLog("[MidiRecorder] Failed to create MIDI clip");

    return success;
}

//==============================================================================
// MidiKeyboardStateListener Implementation

void MidiRecorder::handleNoteOn(juce::MidiKeyboardState* source, int midiChannel,
                                int midiNoteNumber, float velocity)
{
    if (!recording)
        return;

    auto message = MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    recordMidiMessage(message);

    Logger::writeToLog("[MidiRecorder] NOTE ON:  Note=" + String(midiNoteNumber) +
                      " Velocity=" + String(velocity, 2) +
                      " Channel=" + String(midiChannel));
}

void MidiRecorder::handleNoteOff(juce::MidiKeyboardState* source, int midiChannel,
                                 int midiNoteNumber, float velocity)
{
    if (!recording)
        return;

    auto message = MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity);
    recordMidiMessage(message);

    Logger::writeToLog("[MidiRecorder] NOTE OFF: Note=" + String(midiNoteNumber) +
                      " Velocity=" + String(velocity, 2) +
                      " Channel=" + String(midiChannel));
}

//==============================================================================
// Internal Methods

void MidiRecorder::attachToHardwareMidiDevices(te::Edit& edit)
{
    int deviceCount = 0;

    for (const auto& midiIn : engine.getDeviceManager().getMidiInDevices())
    {
        if (midiIn->isEnabled())
        {
            midiIn->keyboardState.addListener(this);
            attachedSources.push_back(&midiIn->keyboardState);
            deviceCount++;

            Logger::writeToLog("[MidiRecorder] Attached to MIDI device: " + midiIn->getName());
        }
    }

    Logger::writeToLog("[MidiRecorder] Attached to " + String(deviceCount) + " hardware MIDI device(s)");
}

void MidiRecorder::detachFromAllSources()
{
    for (auto* source : attachedSources)
    {
        source->removeListener(this);
    }

    attachedSources.clear();
    Logger::writeToLog("[MidiRecorder] Detached from all MIDI sources");
}

bool MidiRecorder::createClipFromRecording(te::Edit& edit)
{
    auto tracks = te::getAudioTracks(edit);

    if (targetTrackIndex < 0 || targetTrackIndex >= tracks.size())
    {
        Logger::writeToLog("[MidiRecorder] Invalid target track index: " + String(targetTrackIndex));
        return false;
    }

    auto* track = tracks[targetTrackIndex];
    Logger::writeToLog("[MidiRecorder] Creating clip on track: " + track->getName());

    // Calculate clip length from recorded sequence
    double clipLengthSeconds = 0.0;
    {
        const ScopedLock sl(recordingLock);
        if (recordedSequence.getNumEvents() > 0)
        {
            // Find the latest event time
            for (int i = 0; i < recordedSequence.getNumEvents(); ++i)
            {
                auto* event = recordedSequence.getEventPointer(i);
                clipLengthSeconds = jmax(clipLengthSeconds, event->message.getTimeStamp());
            }

            // Add a small buffer to ensure the last note-off is included
            clipLengthSeconds += 0.1;
        }
    }

    Logger::writeToLog("[MidiRecorder] Clip length: " + String(clipLengthSeconds, 3) + " seconds");

    // Convert to Tracktion time positions
    auto clipStart = recordingStartPosition;
    auto clipEnd = clipStart + t::TimeDuration::fromSeconds(clipLengthSeconds);

    Logger::writeToLog("[MidiRecorder] Clip position: " +
                      String(clipStart.inSeconds(), 3) + "s to " +
                      String(clipEnd.inSeconds(), 3) + "s");

    // Create MIDI clip with a TimeRange
    auto clipRange = t::TimeRange(clipStart, clipEnd);
    track->insertMIDIClip(track->getName() + " Recording", clipRange, nullptr);

    // Get the clip we just created (should be the last one)
    te::MidiClip* newClip = nullptr;
    for (auto* clip : track->getClips())
    {
        if (auto* midiClip = dynamic_cast<te::MidiClip*>(clip))
        {
            if (midiClip->getPosition().getStart() == clipStart)
            {
                newClip = midiClip;
                break;
            }
        }
    }

    if (!newClip)
    {
        Logger::writeToLog("[MidiRecorder] ERROR: Failed to create MIDI clip");
        return false;
    }

    // Set clip length
    newClip->setLength(clipEnd - clipStart, true);

    // Populate clip with recorded MIDI events
    auto& sequence = newClip->getSequence();
    auto& tempoSequence = edit.tempoSequence;

    int notesAdded = 0;
    {
        const ScopedLock sl(recordingLock);

        for (int i = 0; i < recordedSequence.getNumEvents(); ++i)
        {
            auto* event = recordedSequence.getEventPointer(i);
            const auto& msg = event->message;

            if (msg.isNoteOn())
            {
                // Find corresponding note-off
                double noteStart = event->message.getTimeStamp();
                double noteEnd = noteStart + 0.1; // Default 100ms if no note-off found

                for (int j = i + 1; j < recordedSequence.getNumEvents(); ++j)
                {
                    auto* offEvent = recordedSequence.getEventPointer(j);
                    if (offEvent->message.isNoteOff() &&
                        offEvent->message.getNoteNumber() == msg.getNoteNumber())
                    {
                        noteEnd = offEvent->message.getTimeStamp();
                        break;
                    }
                }

                // Convert time to beat positions
                auto noteStartTime = clipStart + t::TimeDuration::fromSeconds(noteStart);
                auto noteEndTime = clipStart + t::TimeDuration::fromSeconds(noteEnd);

                auto noteStartBeat = tempoSequence.toBeats(noteStartTime);
                auto noteEndBeat = tempoSequence.toBeats(noteEndTime);

                double noteLength = noteEndBeat.inBeats() - noteStartBeat.inBeats();

                // Add note to clip (beat positions are relative to clip start)
                auto clipStartBeat = tempoSequence.toBeats(clipStart);
                double relativeBeatPosition = noteStartBeat.inBeats() - clipStartBeat.inBeats();

                sequence.addNote(msg.getNoteNumber(),
                                t::BeatPosition::fromBeats(relativeBeatPosition),
                                t::BeatDuration::fromBeats(noteLength),
                                static_cast<int>(msg.getFloatVelocity() * 127.0f), // Convert to 0-127
                                0, // color index
                                nullptr); // undo manager

                notesAdded++;
            }
        }
    }

    Logger::writeToLog("[MidiRecorder] Added " + String(notesAdded) + " notes to clip");

    // Clear recorded sequence for next recording
    {
        const ScopedLock sl(recordingLock);
        recordedSequence.clear();
    }

    return notesAdded > 0;
}

void MidiRecorder::recordMidiMessage(const juce::MidiMessage& message)
{
    // Calculate timestamp relative to recording start
    double currentTime = Time::getMillisecondCounterHiRes() / 1000.0;
    double relativeTime = currentTime - recordingStartTime;

    // Create timestamped message
    auto timestampedMessage = message;
    timestampedMessage.setTimeStamp(relativeTime);

    // Add to sequence (thread-safe)
    {
        const ScopedLock sl(recordingLock);
        recordedSequence.addEvent(timestampedMessage);
    }
}
