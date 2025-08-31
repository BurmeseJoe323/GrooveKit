//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef PIANOROLLWINDOW_H
#define PIANOROLLWINDOW_H

#include <juce_gui_basics/juce_gui_basics.h>

class PianoRollWindow : public juce::DocumentWindow {
public:
    explicit PianoRollWindow(int trackIndex);
    ~PianoRollWindow() override;

    int getTrackIndex() const { return trackIndex; }
    // TODO: add setTrackIndex, or just destroy and reopen window?

    void closeButtonPressed() override;
    void activeWindowStatusChanged() override;

private:
    int trackIndex;
};

#endif //PIANOROLLWINDOW_H
