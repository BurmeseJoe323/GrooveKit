#pragma once

#include "../../AppEngine/AppEngine.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;
namespace t = tracktion;

class PlayheadComponent final : public juce::Component, juce::Timer
{
public:
    PlayheadComponent (te::Edit&, EditViewState&);

    void paint (juce::Graphics& g) override;
    bool hitTest (int x, int y) override;
    void setPixelsPerBeat(double p) { pixelsPerBeat = juce::jmax(10.0, p); }
    void setViewStartBeat(t::BeatPosition b) { viewStartBeat = b; }
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;

    te::Edit& edit;
    EditViewState& editViewState;

    int xPosition = 0;
    double pixelsPerBeat = 100.0;
    t::BeatPosition viewStartBeat { t::BeatPosition::fromBeats(0.0) };

    bool firstTimer = true;
};
