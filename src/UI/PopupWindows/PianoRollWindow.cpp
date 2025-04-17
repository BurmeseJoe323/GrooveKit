//
// Created by Joseph Rockwell on 4/8/25.
//

#include "../MainViews/AppView.h"
#include "PianoRollComponents/PianoRollMainComponent.h"

PianoRollWindow::PianoRollWindow() : DocumentWindow("Piano Roll Editor", juce::Colours::darkblue,
                                                    DocumentWindow::closeButton, true) {
    setUsingNativeTitleBar(true);
    setContentOwned(new PianoRollMainComponent(), true);

    setResizable(true, false);
    centreWithSize(getWidth(), getHeight());
}

PianoRollWindow::~PianoRollWindow() = default;

void PianoRollWindow::closeButtonPressed() {
    setVisible(false);
}

void PianoRollWindow::activeWindowStatusChanged() {
    if (isActiveWindow()) {
        if (auto *content = getContentComponent()) {
            content->grabKeyboardFocus();
        }
    }
}
