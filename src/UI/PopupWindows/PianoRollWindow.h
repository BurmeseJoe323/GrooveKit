//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef PIANOROLLWINDOW_H
#define PIANOROLLWINDOW_H

#include <juce_gui_basics/juce_gui_basics.h>

class PianoRollWindow : public juce::DocumentWindow {
public:
    PianoRollWindow();
    ~PianoRollWindow() override;

    void closeButtonPressed() override;
    void activeWindowStatusChanged() override;
};

#endif //PIANOROLLWINDOW_H
