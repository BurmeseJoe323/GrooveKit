//
// Created by Joseph Rockwell on 4/8/25.
//

#include "PianoRollWindow.h"


PianoRollWindow::PianoRollWindow(std::shared_ptr<AppEngine> engine, int trackIndex) :
DocumentWindow("Piano Roll Editor", juce::Colours::darkblue,
        DocumentWindow::closeButton, true),
    trackIndex(trackIndex)
{
    setUsingNativeTitleBar(true);
    editor = std::make_unique<PianoRollMainComponent>(engine, trackIndex);
    setContentOwned(editor.get(), true);

    setResizable(true, false);
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