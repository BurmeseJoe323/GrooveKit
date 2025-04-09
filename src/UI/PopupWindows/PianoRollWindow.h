//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef PIANOROLLWINDOW_H
#define PIANOROLLWINDOW_H
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PianoRollComponents/PianoRoll.h"


class PianoRollWindow : public juce::DocumentWindow {
public:
    PianoRollWindow();
    ~PianoRollWindow() override;

    void closeButtonPressed() override;
private:
    std::unique_ptr<PianoRoll> pianoRoll;
};



#endif //PIANOROLLWINDOW_H
