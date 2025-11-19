#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * @brief Custom MIDI recording implementation that captures MIDI events during recording.
 *
 * MidiRecorder provides a reliable alternative to Tracktion Engine's built-in recording
 * system by directly capturing MIDI messages from MidiKeyboardState objects during
 * recording sessions. It stores timestamped MIDI events and converts them to MIDI clips
 * when recording stops.
 *
 * Usage:
 *  1. Call startRecording() with target track and edit
 *  2. MidiRecorder attaches listeners to hardware and QWERTY keyboard states
 *  3. MIDI events are captured with accurate timestamps
 *  4. Call stopRecording() to create a MIDI clip from the recorded data
 *
 * This approach works around issues in Tracktion Engine's recording API where
 * clips are not created reliably after recording stops.
 */
class MidiRecorder : public juce::MidiKeyboardStateListener
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the MidiRecorder.
     *
     * @param engine Reference to the Tracktion Engine instance.
     */
    explicit MidiRecorder(te::Engine& engine);

    /** Destructor. Ensures recording is stopped and listeners are detached. */
    ~MidiRecorder() override;

    //==============================================================================
    // Recording Control

    /**
     * @brief Starts recording MIDI input to the specified track.
     *
     * Attaches MidiKeyboardStateListener to all enabled MIDI input devices and
     * the QWERTY keyboard state. Begins capturing timestamped MIDI messages.
     * Automatically starts transport playback if not already playing.
     *
     * @param edit The edit containing the target track.
     * @param trackIndex The 0-based index of the track to record to.
     * @param qwertyKeyboardState Optional QWERTY keyboard state to also record from.
     */
    void startRecording(te::Edit& edit, int trackIndex,
                       juce::MidiKeyboardState* qwertyKeyboardState = nullptr);

    /**
     * @brief Stops recording and creates a MIDI clip from the captured data.
     *
     * Detaches all listeners, converts the recorded MIDI buffer to a MidiMessageSequence,
     * creates a MIDI clip on the target track, and positions it correctly on the timeline.
     * Clears the recording buffer after clip creation.
     *
     * @param edit The edit containing the target track.
     * @return True if a clip was successfully created, false if no MIDI was recorded.
     */
    bool stopRecording(te::Edit& edit);

    /**
     * @brief Returns whether recording is currently active.
     *
     * @return True if recording, false otherwise.
     */
    bool isRecording() const { return recording; }

    /**
     * @brief Returns the current recording preview clip bounds.
     *
     * Used for real-time visual feedback during recording. Returns the time range
     * from when the first MIDI note was captured to the current transport position.
     * Returns an empty range if no notes have been recorded yet.
     *
     * @return TimeRange representing the preview clip bounds, or empty if no notes recorded.
     */
    tracktion::TimeRange getPreviewClipBounds() const;

    /**
     * @brief Returns the track index currently being recorded to.
     *
     * @return Track index, or -1 if not recording.
     */
    int getRecordingTrackIndex() const { return targetTrackIndex; }

    //==============================================================================
    // MidiKeyboardStateListener Implementation

    /**
     * @brief Handles note-on events during recording.
     *
     * Called when a MIDI note-on is received from any attached keyboard state.
     * Stores the note-on message with timestamp in the recording buffer.
     *
     * @param source The keyboard state that triggered the event.
     * @param midiChannel MIDI channel (1-16).
     * @param midiNoteNumber MIDI note number (0-127).
     * @param velocity Note velocity (0.0-1.0).
     */
    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel,
                     int midiNoteNumber, float velocity) override;

    /**
     * @brief Handles note-off events during recording.
     *
     * Called when a MIDI note-off is received from any attached keyboard state.
     * Stores the note-off message with timestamp in the recording buffer.
     *
     * @param source The keyboard state that triggered the event.
     * @param midiChannel MIDI channel (1-16).
     * @param midiNoteNumber MIDI note number (0-127).
     * @param velocity Note velocity (0.0-1.0).
     */
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel,
                      int midiNoteNumber, float velocity) override;

private:
    //==============================================================================
    // Internal Methods

    /**
     * @brief Attaches listener to all enabled MIDI input device keyboard states.
     *
     * @param edit The edit containing the MIDI input devices.
     */
    void attachToHardwareMidiDevices(te::Edit& edit);

    /**
     * @brief Detaches listener from all MIDI keyboard states.
     */
    void detachFromAllSources();

    /**
     * @brief Converts the recorded MIDI buffer to a clip and adds it to the track.
     *
     * @param edit The edit containing the target track.
     * @return True if clip was created successfully.
     */
    bool createClipFromRecording(te::Edit& edit);

    /**
     * @brief Records a MIDI message with the current timestamp.
     *
     * @param message The MIDI message to record.
     */
    void recordMidiMessage(const juce::MidiMessage& message);

    /**
     * @brief Handles loop wraparound by clearing the recording buffer.
     *
     * Called when the transport loops back to the start while recording.
     * Clears the recorded sequence and active notes to start a fresh loop pass.
     */
    void handleLoopWraparound();

    //==============================================================================
    // Member Variables

    te::Engine& engine;                                      ///< Reference to Tracktion Engine (not owned).
    te::Edit* currentEdit = nullptr;                         ///< Edit being recorded to (not owned).

    std::atomic<bool> recording{false};                      ///< Whether recording is currently active.
    int targetTrackIndex = -1;                               ///< Index of track being recorded to.
    std::vector<std::pair<te::Clip*, bool>> clipsToRestore;  ///< Clips and their original mute states to restore after recording.

    juce::MidiMessageSequence recordedSequence;              ///< Buffer of recorded MIDI messages.
    double recordingStartTime = 0.0;                         ///< Time when recording started (in seconds).
    tracktion::TimePosition recordingStartPosition;          ///< Transport position when recording started.
    tracktion::TimePosition lastRecordedPosition;            ///< Last transport position when MIDI was recorded (for loop detection).
    tracktion::TimePosition firstNotePosition;               ///< Transport position when first MIDI note was captured (for preview clip start).
    bool hasRecordedNotes = false;                           ///< Whether any MIDI notes have been captured yet.

    std::vector<juce::MidiKeyboardState*> attachedSources;   ///< Keyboard states we're listening to.
    std::set<int> activeNotes;                               ///< Currently held notes (note numbers without matching note-offs).

    juce::CriticalSection recordingLock;                     ///< Thread safety for recording buffer.
};
