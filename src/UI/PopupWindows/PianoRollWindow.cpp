//
// Created by Joseph Rockwell on 4/8/25.
//

#include "../MainViews/AppView.h"

PianoRollWindow::PianoRollWindow() : DocumentWindow("Piano Roll Editor", juce::Colours::darkblue,
                                                    DocumentWindow::closeButton, true) {
    // TODO: get window to size properly on opening
    centreWithSize(800, 600);
    setUsingNativeTitleBar(true);
    setResizable(true, false);
    pianoRoll = std::make_unique<PianoRollEditor>();
    pianoRoll->setup(10, 900, 20);
    setContentOwned(pianoRoll.get(), true);
    pianoRoll->setVisible(true);
}

PianoRollWindow::~PianoRollWindow() = default;

void PianoRollWindow::closeButtonPressed() {
    setVisible(false);
}
