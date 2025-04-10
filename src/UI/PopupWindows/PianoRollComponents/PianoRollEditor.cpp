//
// Created by Joseph Rockwell on 4/8/25.
//

#include "PianoRollEditor.h"

PianoRollEditor::PianoRollEditor() {
    setSize(800, 600);

    // Setup note grid
    // TODO: add to viewport to be scrollable
    addAndMakeVisible(noteGrid);

    // Setup keyboard
    // TODO: add to viewport to be scrollable
    addAndMakeVisible(keyboard);

}

void PianoRollEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::darkgrey.darker());
}
