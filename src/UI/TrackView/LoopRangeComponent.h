#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace t = tracktion;
namespace te = tracktion::engine;

/**
 * Component that draws the loop range start and end lines over the track area.
 * Invisible when no loop is active.
 */
class LoopRangeComponent final : public juce::Component
{
public:
    LoopRangeComponent(te::Edit& edit);

    void paint(juce::Graphics& g) override;

    void setPixelsPerSecond(double pps);
    void setViewStart(t::TimePosition t);
    void setLoopRange(t::TimeRange range);
    void setLooping(bool shouldLoop);

private:
    te::Edit& edit;
    double pixelsPerSecond = 100.0;
    t::TimePosition viewStart = t::TimePosition::fromSeconds(0.0);
    t::TimeRange loopRange;
    bool looping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopRangeComponent)
};
