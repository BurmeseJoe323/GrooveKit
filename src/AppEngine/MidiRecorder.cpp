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

    // Mute all clips on the track to prevent playback during recording
    // (but keep the track itself unmuted so live input can be heard)
    auto tracks = te::getAudioTracks(edit);
    if (trackIndex >= 0 && trackIndex < tracks.size())
    {
        auto* track = tracks[trackIndex];
        clipsToRestore.clear();

        for (auto* clip : track->getClips())
        {
            bool wasClipMuted = clip->isMuted();
            clipsToRestore.push_back({clip, wasClipMuted});
            clip->setMuted(true);
        }

        Logger::writeToLog("[MidiRecorder] Muted " + String(clipsToRestore.size()) +
                          " clip(s) on track during recording");
    }

    // Clear previous recording buffer
    {
        const ScopedLock sl(recordingLock);
        recordedSequence.clear();
        activeNotes.clear();
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

    // If looping is enabled, always record from loop start position
    if (transport.looping)
    {
        recordingStartPosition = transport.getLoopRange().getStart();
        Logger::writeToLog("[MidiRecorder] Loop recording - using loop start position: " +
                          String(recordingStartPosition.inSeconds()) + " seconds");
    }
    else
    {
        recordingStartPosition = transport.getPosition();
        Logger::writeToLog("[MidiRecorder] Recording start position: " +
                          String(recordingStartPosition.inSeconds()) + " seconds");
    }

    // Initialize last recorded position for loop detection
    lastRecordedPosition = recordingStartPosition;

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

    // Handle any notes that are still being held down
    // Synthesize note-off events at the current transport position
    std::set<int> heldNotes;
    {
        const ScopedLock sl(recordingLock);
        heldNotes = activeNotes;  // Copy the set
    }

    if (!heldNotes.empty())
    {
        Logger::writeToLog("[MidiRecorder] Synthesizing note-offs for " +
                          String(heldNotes.size()) + " held note(s)");

        auto& transport = edit.getTransport();
        auto currentPosition = transport.getPosition();
        auto timeSinceRecordingStart = currentPosition - recordingStartPosition;
        double relativeTime = timeSinceRecordingStart.inSeconds();

        for (int noteNumber : heldNotes)
        {
            // Add note-off to the recording sequence at current position
            auto noteOffMessage = juce::MidiMessage::noteOff(1, noteNumber, 0.0f);
            noteOffMessage.setTimeStamp(relativeTime);

            {
                const ScopedLock sl(recordingLock);
                recordedSequence.addEvent(noteOffMessage);
            }

            Logger::writeToLog("[MidiRecorder] Synthesized note-off for note " + String(noteNumber));
        }
    }

    // Send note-offs to all attached keyboard states to stop any hanging notes in the synth
    for (auto* keyboardState : attachedSources)
    {
        if (keyboardState != nullptr)
        {
            for (int noteNumber : heldNotes)
            {
                keyboardState->noteOff(1, noteNumber, 0.0f);
            }
        }
    }

    // Clear active notes tracking
    {
        const ScopedLock sl(recordingLock);
        activeNotes.clear();
    }

    // Restore clips' original mute states
    for (auto& [clip, wasMuted] : clipsToRestore)
    {
        clip->setMuted(wasMuted);
    }
    Logger::writeToLog("[MidiRecorder] Restored mute state for " +
                      String(clipsToRestore.size()) + " clip(s)");
    clipsToRestore.clear();

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

tracktion::TimeRange MidiRecorder::getPreviewClipBounds() const
{
    if (!recording || !currentEdit)
        return tracktion::TimeRange();

    auto& transport = currentEdit->getTransport();
    auto currentPos = transport.getPosition();

    // Detect loop wraparound proactively (even if no MIDI is being played)
    // This ensures the buffer clears when looping, even during silence
    if (transport.looping)
    {
        auto loopRange = transport.getLoopRange();

        // Check if we wrapped from near the end back to near the start
        if (lastRecordedPosition > loopRange.getStart() &&
            currentPos < lastRecordedPosition)
        {
            // Loop wraparound detected - clear the buffer
            // Need to cast away const since we're modifying state, but this is
            // called from UI thread at 30Hz so it's safe
            const_cast<MidiRecorder*>(this)->handleLoopWraparound();
        }
    }

    // Update last position (cast away const - safe because called from UI thread)
    const_cast<MidiRecorder*>(this)->lastRecordedPosition = currentPos;

    // Return range from recording start position to current transport position
    return tracktion::TimeRange(recordingStartPosition, currentPos);
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

    // Track this note as active (held down)
    {
        const ScopedLock sl(recordingLock);
        activeNotes.insert(midiNoteNumber);
    }

    Logger::writeToLog("[MidiRecorder] NOTE ON:  Note=" + String(midiNoteNumber) +
                      " Velocity=" + String(velocity, 2) +
                      " Channel=" + String(midiChannel));
}

void MidiRecorder::handleNoteOff(juce::MidiKeyboardState* source, int midiChannel,
                                 int midiNoteNumber, float velocity)
{
    if (!recording)
        return;

    // Check if this note-off has a corresponding note-on in the buffer
    // If not, discard it (this happens when holding notes across loop boundary)
    bool hasMatchingNoteOn = false;
    {
        const ScopedLock sl(recordingLock);
        hasMatchingNoteOn = activeNotes.count(midiNoteNumber) > 0;
    }

    if (!hasMatchingNoteOn)
    {
        Logger::writeToLog("[MidiRecorder] Discarding orphaned NOTE OFF (no matching note-on): Note=" +
                          String(midiNoteNumber));
        return;
    }

    auto message = MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity);
    recordMidiMessage(message);

    // Remove this note from active set
    {
        const ScopedLock sl(recordingLock);
        activeNotes.erase(midiNoteNumber);
    }

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

    // Use the EXACT preview bounds: from recording start to current transport position
    // This ensures the created clip matches the preview clip perfectly
    auto& transport = edit.getTransport();
    auto clipStart = recordingStartPosition;
    auto clipEnd = transport.getPosition();

    Logger::writeToLog("[MidiRecorder] Clip position: " +
                      String(clipStart.inSeconds(), 3) + "s to " +
                      String(clipEnd.inSeconds(), 3) + "s");

    // Find ALL clips that overlap with our recording range and delete them
    auto clipRange = t::TimeRange(clipStart, clipEnd);
    std::vector<te::Clip*> clipsToDelete;

    for (auto* clip : track->getClips())
    {
        if (auto* midiClip = dynamic_cast<te::MidiClip*>(clip))
        {
            auto clipPos = midiClip->getPosition();
            auto existingRange = t::TimeRange(clipPos.getStart(), clipPos.getEnd());

            // Check if the recording range overlaps with this clip
            if (existingRange.overlaps(clipRange))
            {
                clipsToDelete.push_back(midiClip);
                Logger::writeToLog("[MidiRecorder] Found overlapping clip to delete: " + midiClip->getName());
            }
        }
    }

    // Delete all overlapping clips
    for (auto* clipToDelete : clipsToDelete)
    {
        clipToDelete->removeFromParent();
    }

    if (!clipsToDelete.empty())
    {
        Logger::writeToLog("[MidiRecorder] Deleted " + String(clipsToDelete.size()) +
                          " overlapping clip(s)");
    }

    // Create a new clip for the recording
    Logger::writeToLog("[MidiRecorder] Creating new MIDI clip");
    track->insertMIDIClip(track->getName() + " Recording", clipRange, nullptr);

    // Find the newly created clip
    te::MidiClip* targetClip = nullptr;
    for (auto* clip : track->getClips())
    {
        if (auto* midiClip = dynamic_cast<te::MidiClip*>(clip))
        {
            if (midiClip->getPosition().getStart() == clipStart)
            {
                targetClip = midiClip;
                break;
            }
        }
    }

    if (!targetClip)
    {
        Logger::writeToLog("[MidiRecorder] ERROR: Failed to create MIDI clip");
        return false;
    }

    // Populate clip with recorded MIDI events
    auto& sequence = targetClip->getSequence();
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

    // Always return true if we created a clip (even if empty)
    // This matches the preview behavior - clip exists from first note position to end
    return true;
}

void MidiRecorder::handleLoopWraparound()
{
    Logger::writeToLog("[MidiRecorder] Loop wraparound detected - clearing recording buffer");

    // Clear the buffer to start a fresh recording pass
    const ScopedLock sl(recordingLock);
    recordedSequence.clear();
    activeNotes.clear();  // Clear held notes from previous loop pass
}

void MidiRecorder::recordMidiMessage(const juce::MidiMessage& message)
{
    if (!currentEdit)
        return;

    auto& transport = currentEdit->getTransport();
    auto currentPosition = transport.getPosition();

    // Detect loop wraparound: if looping is enabled and the position jumped backwards significantly
    if (transport.looping)
    {
        auto loopRange = transport.getLoopRange();

        // Check if we wrapped from near the end back to near the start
        if (lastRecordedPosition > loopRange.getStart() &&
            currentPosition < lastRecordedPosition)
        {
            handleLoopWraparound();
        }
    }

    lastRecordedPosition = currentPosition;

    // Calculate timestamp based on transport position, not wall-clock time
    // This ensures notes are positioned correctly even after loop wraparound
    auto timeSinceRecordingStart = currentPosition - recordingStartPosition;
    double relativeTime = timeSinceRecordingStart.inSeconds();

    // Create timestamped message
    auto timestampedMessage = message;
    timestampedMessage.setTimeStamp(relativeTime);

    // Add to sequence (thread-safe)
    {
        const ScopedLock sl(recordingLock);
        recordedSequence.addEvent(timestampedMessage);
    }
}
