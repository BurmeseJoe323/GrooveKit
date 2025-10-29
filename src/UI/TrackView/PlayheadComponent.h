#pragma once

#include "../../AppEngine/AppEngine.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

class PlayheadComponent final : public juce::Component, juce::Timer
{
public:
    PlayheadComponent (te::Edit&, EditViewState&);

    void paint (juce::Graphics& g) override;
    bool hitTest (int x, int y) override;
    void setPixelsPerSecond(double p) { pixelsPerSecond = juce::jmax(10.0, p); }
    void setViewStart(te::TimePosition t) { viewStart = t; }
    // void mouseDrag (const juce::MouseEvent&) override;
    // void mouseDown (const juce::MouseEvent&) override;
    // void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    te::Edit& edit;
    EditViewState& editViewState;

    int xPosition = 0;
    double pixelsPerSecond = 100.0;
    te::TimePosition viewStart { te::TimePosition::fromSeconds(0.0) };

    bool firstTimer = true;
};
