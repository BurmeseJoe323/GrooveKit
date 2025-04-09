//
// Created by Joseph Rockwell on 4/8/25.
//

#include "PianoRoll.h"

PianoRoll::PianoRoll() {
    setBounds(0, 0, 800, 450);
}

void PianoRoll::paint(juce::Graphics &g) {
    g.setColour(juce::Colours::darkolivegreen);
    g.drawText("This is a text.", getLocalBounds(), juce::Justification::centred);
}
