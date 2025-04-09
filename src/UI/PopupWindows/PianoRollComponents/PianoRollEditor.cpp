//
// Created by Joseph Rockwell on 4/8/25.
//

#include "PianoRollEditor.h"

PianoRollEditor::PianoRollEditor() {
    setBounds(getBoundsInParent());
}

void PianoRollEditor::paint(juce::Graphics &g) {
    g.setColour(juce::Colours::darkolivegreen);
    g.drawText("This is a text.", getLocalBounds(), juce::Justification::centred);
}
