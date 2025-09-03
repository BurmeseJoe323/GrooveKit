//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef PIANOROLLWINDOW_H
#define PIANOROLLWINDOW_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "PianoRollComponents/PianoRollMainComponent.h"
#include "tracktion_graph/tracktion_graph.h"

class PianoRollWindow : public juce::DocumentWindow {
public:
    explicit PianoRollWindow(int trackIndex);
    ~PianoRollWindow() override;

    int getTrackIndex() const { return trackIndex; }
    // TODO: add setTrackIndex, or just destroy and reopen window?

    void loadSequence(juce::Array<tracktion::MidiNote *> notes);

    void closeButtonPressed() override;
    void activeWindowStatusChanged() override;

private:
    int trackIndex;
    std::unique_ptr<PianoRollMainComponent> editor;
};

#endif //PIANOROLLWINDOW_H
