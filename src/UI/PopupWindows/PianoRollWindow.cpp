//
// Created by Joseph Rockwell on 4/8/25.
//

#include "../MainViews/AppView.h"

PianoRollWindow::PianoRollWindow() : DocumentWindow("Piano Roll Editor", juce::Colours::darkblue,
                                                    DocumentWindow::closeButton, true) {
    setUsingNativeTitleBar(true);
    pianoRoll = std::make_unique<PianoRollEditor>();
    pianoRoll->setup(10, 900, 20);
    pianoRoll->setVisible(true);
    setContentOwned(pianoRoll.get(), true);

    setResizable(true, false);
    centreWithSize(800, 600);
}

PianoRollWindow::~PianoRollWindow() = default;

void PianoRollWindow::closeButtonPressed() {
    setVisible(false);
}
