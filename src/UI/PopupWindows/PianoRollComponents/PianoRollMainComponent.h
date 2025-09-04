//
// Created by Joseph Rockwell on 4/14/25.
//

#ifndef MAINCOMPONENT_H
#define MAINCOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "PianoRollEditor.h"
#include "tracktion_engine/tracktion_engine.h"

class PianoRollMainComponent : public juce::Component, juce::Timer {
    public:
    PianoRollMainComponent();
    ~PianoRollMainComponent() override = default;

    void resized() override;

    void timerCallback() override;

    void loadSequence(const te::MidiList &notes);

private:
    st_int tickTest;
    PianoRollEditor editor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollMainComponent)
};



#endif //MAINCOMPONENT_H
