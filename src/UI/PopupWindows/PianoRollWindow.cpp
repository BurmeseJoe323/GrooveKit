//
// Created by Joseph Rockwell on 4/8/25.
//

#include "../MainViews/AppView.h"

PianoRollWindow::PianoRollWindow() : DocumentWindow("Piano Roll Editor", juce::Colours::darkblue,
                                                    DocumentWindow::closeButton, true) {
    centreWithSize(800, 450);
    setUsingNativeTitleBar(true);
    setResizable(true, false);
    setContentOwned(new PianoRollEditor(), true);
    setVisible(true);
}

PianoRollWindow::~PianoRollWindow() = default;

void PianoRollWindow::closeButtonPressed() {
    setVisible(false);
}

