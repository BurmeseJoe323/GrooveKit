//
// Created by Joseph Rockwell on 4/8/25.
//

#include "../MainViews/AppView.h"

PianoRollWindow::PianoRollWindow() : DocumentWindow("Piano Roll Editor", juce::Colours::darkblue,
                                                    DocumentWindow::closeButton, true) {
    centreWithSize(300, 300);
    setVisible(true);
    setUsingNativeTitleBar(true);
    setResizable(true, false);
    // pianoRoll = std::make_unique<PianoRoll>();
}

PianoRollWindow::~PianoRollWindow() = default;

void PianoRollWindow::closeButtonPressed() {
    setVisible(false);
}

