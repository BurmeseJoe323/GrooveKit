//
// Created by Joseph Rockwell on 4/21/25.
//

#ifndef PLAYHEADCOMPONENT_H
#define PLAYHEADCOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "../../AppEngine/AppEngine.h"

namespace te = tracktion;

class PlayheadComponent : public juce::Component, private juce::Timer
{
public:
    PlayheadComponent (te::Edit&, EditViewState&);

    void paint (juce::Graphics& g) override;
    bool hitTest (int x, int y) override;
    // void mouseDrag (const juce::MouseEvent&) override;
    // void mouseDown (const juce::MouseEvent&) override;
    // void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    te::Edit& edit;
    EditViewState& editViewState;

    int xPosition = 0;
    bool firstTimer = true;
};



#endif //PLAYHEADCOMPONENT_H
