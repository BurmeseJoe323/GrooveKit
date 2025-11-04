#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <tracktion_engine/tracktion_engine.h>
#include <memory>

namespace te = tracktion::engine;

class AppEngine;

/**
 * MidiListener handles MIDI keyboard input and routes it to the armed track.
 * It manages the MidiKeyboardState and provides QWERTY-to-MIDI mapping functionality.
 */
class MidiListener final : public juce::MidiKeyboardStateListener
{
public:
    explicit MidiListener(std::shared_ptr<AppEngine> engine);
    ~MidiListener() override;

    // MidiKeyboardStateListener overrides
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    /**
     * Returns a reference to the MidiKeyboardState managed by this listener.
     */
    juce::MidiKeyboardState& getMidiKeyboardState();

    /**
     * Handles keyboard state changes for QWERTY-to-MIDI mapping.
     * @param isKeyDown Whether a key is pressed or released
     * @return true if the key event was handled
     */
    bool handleKeyStateChanged(bool isKeyDown);

    /**
     * Handles key press events for octave adjustment (Z/X keys).
     * @param keyPress The key press event
     * @return true if the key event was handled
     */
    bool handleKeyPress(const juce::KeyPress& keyPress);

    /**
     * Gets the array of keys used for QWERTY-to-MIDI mapping.
     */
    const juce::Array<char>& getNoteKeys() const { return noteKeys; }

private:
    void injectNoteMessage(const juce::MidiMessage& msg);

    std::shared_ptr<AppEngine> appEngine;
    juce::MidiKeyboardState midiKeyboardState;

    // QWERTY mapping keys (A-L for chromatic scale)
    juce::Array<char> noteKeys{ 'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G', 'Y', 'H', 'U', 'J', 'K', 'O', 'L' };

    // Base octave for QWERTY mapping; adjusted with Z/X keys
    int keyboardBaseOctave = 4;
};
