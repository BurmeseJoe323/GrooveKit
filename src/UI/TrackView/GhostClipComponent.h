// Written by Claude Code
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;
namespace t = tracktion;

/**
 * Ghost preview component showing where a dragged clip will land.
 * Displays a semi-transparent outline with color-coding for valid/invalid drops.
 * Written by Claude Code
 */
class GhostClipComponent final : public juce::Component
{
public:
    GhostClipComponent();

    void paint (juce::Graphics& g) override;

    // Update ghost position and validity
    void setDropLocation (int trackIndex, t::TimePosition time, t::TimeDuration length, bool isValid);

    // Show/hide the ghost
    void show();
    void hide();

private:
    bool isValidDrop = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GhostClipComponent)
};
