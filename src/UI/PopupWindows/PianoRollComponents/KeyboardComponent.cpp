//
// Created by Joseph Rockwell on 4/8/25.
//

#include "KeyboardComponent.h"

KeyboardComponent::KeyboardComponent() {
    blackPitches = {1, 3, 6, 8, 10};
}

void KeyboardComponent::paint(juce::Graphics &g) {
    const float noteCompHeight = getHeight() / 128.0;
    float line = 0; // noteCompHeight;

    // Draw keys first
    for (int i = 127; i >= 0; i--) {
        const int pitch = i % 12;
        g.setColour(blackPitches.contains(pitch) ? (juce::Colours::black) : juce::Colours::white.darker(0.1));

        // "Cast" to int using JUCE's floorAsInt function
        g.fillRect(0, juce::detail::floorAsInt(line), getWidth(), juce::detail::floorAsInt(noteCompHeight));

        line += noteCompHeight;
    }

    // Draw lines in between keys
    line = 0;
    for (int i = 127; i >= 0; i--) {
        g.setColour(juce::Colours::black);
        g.drawLine(0, floor(line), getWidth(), floor(line)); // Floor these values to be consistent with keys

        line += noteCompHeight;
    }
}
