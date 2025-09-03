//
// Created by Joseph Rockwell on 4/8/25.
//

#include "PianoRollWindow.h"


PianoRollWindow::PianoRollWindow(int trackIndex) : DocumentWindow("Piano Roll Editor", juce::Colours::darkblue,
                                                    DocumentWindow::closeButton, true) {
    setUsingNativeTitleBar(true);
    editor = std::make_unique<PianoRollMainComponent>();
    setContentOwned(editor.get(), true);

    setResizable(true, false);
    this->trackIndex = trackIndex;
}

PianoRollWindow::~PianoRollWindow() = default;

void PianoRollWindow::loadSequence(juce::Array<tracktion::MidiNote *> notes) { editor->loadSequence(notes); }

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
