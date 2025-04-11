//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef PIANOROLL_H
#define PIANOROLL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "KeyboardComponent.h"
#include "NoteGridComponent.h"
#include "TimelineComponent.h"

class PianoRollEditor : public juce::Component {
public:
    PianoRollEditor();

    ~PianoRollEditor() override = default;

    void paint(juce::Graphics &g) override;

private:
    /* The three primary components of the piano roll:
     * the keyboard visualizer, the piano roll grid,
     * and the timeline
     */
    // TODO: implement timeline component
    KeyboardComponent keyboard; // Keyboard visualizer
    NoteGridComponent noteGrid; // Piano roll interface where notes will be set
    TimelineComponent timeline; // Piano roll interface where notes will be set

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollEditor)
};


#endif //PIANOROLL_H
