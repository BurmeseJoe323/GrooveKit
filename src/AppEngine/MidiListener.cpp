#include "MidiListener.h"
#include "AppEngine.h"

MidiListener::MidiListener(AppEngine* engine)
    : appEngine(engine)
{
    midiKeyboardState.addListener(this);
}

MidiListener::~MidiListener()
{
    midiKeyboardState.removeListener(this);
}

juce::MidiKeyboardState& MidiListener::getMidiKeyboardState()
{
    return midiKeyboardState;
}

void MidiListener::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    // Translate MidiKeyboardState events into JUCE MidiMessages and forward them to the selected track
    const int ch = (midiChannel <= 0 ? 1 : midiChannel);
    const juce::uint8 vel = static_cast<juce::uint8>(juce::jlimit(0, 127, juce::roundToInt(velocity * 127.0f)));
    juce::MidiMessage on = juce::MidiMessage::noteOn(ch, juce::jlimit(0, 127, midiNoteNumber), vel);
    injectNoteMessage(on);
}

void MidiListener::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/)
{
    // Corresponding note-off routed to the engine
    const int ch = (midiChannel <= 0 ? 1 : midiChannel);
    juce::MidiMessage off = juce::MidiMessage::noteOff(ch, midiNoteNumber);
    injectNoteMessage(off);
}

void MidiListener::injectNoteMessage(const juce::MidiMessage& msg)
{
    // Send the live MIDI message to the currently selected track so its instrument plays immediately
    if (!appEngine)
        return;

    te::AudioTrack* track = appEngine->getArmedTrack();
    if (track == nullptr)
        return;

    // Ensure the Tracktion Edit has an allocated playback context before injecting live MIDI
    track->edit.getTransport().ensureContextAllocated();

    // Use a fixed MPESourceID for live input; this keeps voices grouped if using MPE instruments
    const te::MPESourceID source((juce::uint8)2);
    track->injectLiveMidiMessage(te::MidiMessageWithSource(msg, source));
}

bool MidiListener::handleKeyStateChanged(bool isKeyDown)
{
    int noteNumber = keyboardBaseOctave * 12; // Starts at the C of whatever octave is set
    for (const char key : noteKeys)
    {
        if (isKeyDown)
        {
            if (juce::KeyPress::isKeyCurrentlyDown(key)
                && !midiKeyboardState.isNoteOn(1, noteNumber))
            {
                midiKeyboardState.noteOn(1, noteNumber, 0.5f);
                return true;
            }
        }
        else if (!juce::KeyPress::isKeyCurrentlyDown(key)
                 && midiKeyboardState.isNoteOn(1, noteNumber))
        {
            midiKeyboardState.noteOff(1, noteNumber, 0.5f);
            return true;
        }
        noteNumber++;
    }

    return false;
}

bool MidiListener::handleKeyPress(const juce::KeyPress& keyPress)
{
    // Z/X change the base octave for QWERTY-to-MIDI mapping
    if (keyPress.isKeyCode('Z'))
    {
        keyboardBaseOctave = juce::jlimit(0, 10, keyboardBaseOctave - 1);
        midiKeyboardState.allNotesOff(1); // Turn off all MIDI notes that may be held down
        return true;
    }
    if (keyPress.isKeyCode('X'))
    {
        keyboardBaseOctave = juce::jlimit(0, 10, keyboardBaseOctave + 1);
        midiKeyboardState.allNotesOff(1); // Turn off all MIDI notes that may be held down
        return true;
    }

    const auto c = (char) keyPress.getTextCharacter();
    static const juce::String noteKeySet = "awsedf tgyhujko l"; // your mapping (with spaces if you had them)
    if (noteKeySet.containsChar (juce::CharacterFunctions::toLowerCase (c)))
        return true;

    return false;
}
