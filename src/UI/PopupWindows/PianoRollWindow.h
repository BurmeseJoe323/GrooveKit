//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef PIANOROLLWINDOW_H
#define PIANOROLLWINDOW_H

#include "../../AppEngine/AppEngine.h"
#include "PianoRollComponents/PianoRollMainComponent.h"
#include "tracktion_graph/tracktion_graph.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

class PianoRollWindow : public juce::DocumentWindow
{
public:
    explicit PianoRollWindow (AppEngine& engine, int trackIndex);
    ~PianoRollWindow() override;

    int getTrackIndex() const { return trackIndex; }
    void setTrackIndex (int newTrackIndex);

    void closeButtonPressed() override;
    void activeWindowStatusChanged() override;

private:
    int trackIndex;
    std::unique_ptr<PianoRollMainComponent> editor;
    AppEngine& engine;
};
#endif //PIANOROLLWINDOW_H