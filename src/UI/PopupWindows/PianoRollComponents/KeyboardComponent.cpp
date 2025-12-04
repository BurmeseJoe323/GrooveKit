//
// Created by Joseph Rockwell on 4/8/25.
//

#include "KeyboardComponent.h"

//==============================================================================
// Construction / Destruction
//==============================================================================

KeyboardComponent::KeyboardComponent() {
    blackPitches = {1, 3, 6, 8, 10};
}

//==============================================================================
// Component Overrides
//==============================================================================

void KeyboardComponent::paint(juce::Graphics &g) {
    // For drum tracks, only show 16 keys (MIDI 36-51)
    // For instrument tracks, show all 128 keys (MIDI 0-127)
    const int startNote = isDrumTrack ? 51 : 127;
    const int endNote = isDrumTrack ? 36 : 0;

    // Use the noteHeight synchronized from NoteGridComponent
    const float noteCompHeight = noteHeight;
    float line = 0;

    // Draw keys first
    for (int i = startNote; i >= endNote; i--) {
        const int pitch = i % 12;

        // Determine key color (Written by Claude Code)
        juce::Colour keyColor = blackPitches.contains(pitch)
            ? juce::Colours::black
            : juce::Colours::white.darker(0.1);

        g.setColour(keyColor);

        // "Cast" to int using JUCE's floorAsInt function
        g.fillRect(0, juce::detail::floorAsInt(line), getWidth(), juce::detail::floorAsInt(noteCompHeight));

        // Draw note number at end of note
        g.setColour(blackPitches.contains(pitch) ? juce::Colours::white.darker(0.1) : (juce::Colours::black));
        if (i % 12 == 0)
        {
            g.drawFittedText("C" + juce::String((i - 24) / 12), getWidth() - 30, juce::detail::floorAsInt(line) + juce::detail::floorAsInt(noteCompHeight) / 2,
                       25, juce::detail::floorAsInt(noteCompHeight) / 4, juce::Justification::left, false);
        }

        line += noteCompHeight;
    }

    // Draw lines in between keys
    line = 0;
    for (int i = startNote; i >= endNote; i--) {
        g.setColour(juce::Colours::black);
        g.drawLine(0, floor(line), getWidth(), floor(line)); // Floor these values to be consistent with keys

        line += noteCompHeight;
    }
}