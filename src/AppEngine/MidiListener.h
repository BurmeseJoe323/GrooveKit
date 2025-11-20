#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

class AppEngine;

/**
 * @brief Handles QWERTY keyboard input as a MIDI controller for live playing and recording.
 *
 * MidiListener provides computer keyboard (QWERTY) support for playing virtual instruments
 * in GrooveKit. It manages a MidiKeyboardState that tracks which notes are currently held,
 * and injects MIDI messages to the armed track for both live monitoring and recording.
 *
 * **Ownership**: Owned by AppEngine (lifetime matches AppEngine, not UI components).
 *
 * **Thread Safety**: MIDI callbacks execute on MIDI input thread. All Tracktion Engine
 * operations are marshaled to message thread via MessageManager::callAsync().
 *
 * **Hardware MIDI**: External MIDI controllers are handled separately by Tracktion's
 * InputDevice system (see AudioEngine::enableAllMidiInputs()).
 *
 * **QWERTY Mapping**:
 * - Note keys: A W S E D F T G Y H U J K O L (chromatic scale, white/black interleaved)
 * - Octave down: Z key (decreases keyboardBaseOctave)
 * - Octave up: X key (increases keyboardBaseOctave)
 * - Default octave: 4 (middle C = C4)
 *
 * @see MidiRecorder for MIDI recording implementation
 * @see AudioEngine::enableAllMidiInputs() for hardware MIDI setup
 */
class MidiListener final : public juce::MidiKeyboardStateListener
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the MidiListener and attaches it to the AppEngine.
     *
     * @param engine Pointer to AppEngine (not owned, must outlive this object).
     */
    explicit MidiListener(AppEngine* engine);

    /** Destructor. Removes listener from keyboard state. */
    ~MidiListener() override;

    //==============================================================================
    // MidiKeyboardStateListener Implementation

    /**
     * @brief Handles note-on events from QWERTY keyboard.
     *
     * **CRITICAL**: This callback runs on MIDI input thread, NOT message thread.
     * All Tracktion Engine operations are marshaled to message thread using
     * MessageManager::callAsync() to prevent race conditions.
     *
     * @param source The keyboard state that triggered the event (unused).
     * @param midiChannel MIDI channel (1-16).
     * @param midiNoteNumber MIDI note number (0-127).
     * @param velocity Note velocity (0.0-1.0).
     */
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    /**
     * @brief Handles note-off events from QWERTY keyboard.
     *
     * **CRITICAL**: This callback runs on MIDI input thread, NOT message thread.
     * All Tracktion Engine operations are marshaled to message thread using
     * MessageManager::callAsync() to prevent race conditions.
     *
     * @param source The keyboard state that triggered the event (unused).
     * @param midiChannel MIDI channel (1-16).
     * @param midiNoteNumber MIDI note number (0-127).
     * @param velocity Note velocity (0.0-1.0).
     */
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    //==============================================================================
    // Public Interface

    /**
     * @brief Returns a reference to the MidiKeyboardState managed by this listener.
     *
     * Used by MidiRecorder to attach additional listeners for recording.
     *
     * @return Reference to internal MidiKeyboardState.
     */
    juce::MidiKeyboardState& getMidiKeyboardState();

    /**
     * @brief Handles keyboard state changes for QWERTY-to-MIDI mapping.
     *
     * Called by UI components when computer keyboard state changes.
     * Triggers note-on/off based on current key states and noteKeys mapping.
     *
     * @param isKeyDown Whether a key is pressed (true) or released (false).
     * @return True if a key event was handled.
     */
    bool handleKeyStateChanged(bool isKeyDown);

    /**
     * @brief Handles key press events for octave adjustment (Z/X keys).
     *
     * - Z key: Decrease octave (keyboardBaseOctave -= 1)
     * - X key: Increase octave (keyboardBaseOctave += 1)
     *
     * @param keyPress The key press event.
     * @return True if the key event was handled (Z or X key).
     */
    bool handleKeyPress(const juce::KeyPress& keyPress);

    /**
     * @brief Gets the array of keys used for QWERTY-to-MIDI mapping.
     *
     * @return Array of characters representing the chromatic note mapping.
     */
    const juce::Array<char>& getNoteKeys() const { return noteKeys; }

private:
    //==============================================================================
    // Internal Methods

    /**
     * @brief Injects a MIDI message to the armed track's live input.
     *
     * Routes MIDI to MidiRecorder (if recording) or directly to armed track plugin
     * for live monitoring. Called from message thread after marshaling.
     *
     * @param msg The MIDI message to inject.
     */
    void injectNoteMessage(const juce::MidiMessage& msg);

    //==============================================================================
    // Member Variables

    AppEngine* appEngine;                          ///< Pointer to AppEngine (not owned).
    juce::MidiKeyboardState midiKeyboardState;     ///< Tracks currently held QWERTY notes.

    juce::Array<char> noteKeys{                    ///< QWERTY-to-MIDI note mapping (chromatic scale).
        'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G', 'Y', 'H', 'U', 'J', 'K', 'O', 'L'
    };

    int keyboardBaseOctave = 4;                    ///< Base octave for QWERTY mapping (adjusted with Z/X keys).
};
