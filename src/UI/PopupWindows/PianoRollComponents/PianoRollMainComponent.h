//
// Created by Joseph Rockwell on 4/14/25.
//

#ifndef MAINCOMPONENT_H
#define MAINCOMPONENT_H

#include "../../../AppEngine/AppEngine.h"
#include "PianoRollEditor.h"
#include "tracktion_engine/tracktion_engine.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Acts a main component as in a JUCE app.
 */
class PianoRollMainComponent : public juce::Component, juce::Timer
{
public:
    PianoRollMainComponent (AppEngine& engine, int trackIndex);
    ~PianoRollMainComponent() override = default;

    void resized() override;

    void timerCallback() override;

private:
    st_int tickTest;
    PianoRollEditor editor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PianoRollMainComponent)
};
#endif //MAINCOMPONENT_H