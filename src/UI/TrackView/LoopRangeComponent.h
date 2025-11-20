#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace t  = tracktion;
namespace te = tracktion::engine;

/**
 * @class LoopRangeComponent
 *
 * @brief Draws visual start/end loop markers across the track view.
 *
 * This component is meant to sit on top of the track area (usually inside
 * a Viewport and aligned with the Timeline). It is purely visual:
 *
 *  • Draws two vertical lines representing the loop range start and end
 *  • Uses beat-based positioning (pixelsPerBeat + viewStartBeat)
 *  • Does **not** accept mouse input — it’s purely display
 *
 * The TimelineComponent typically owns and updates:
 *  - pixelsPerBeat (zoom level)
 *  - viewStartBeat (scroll offset)
 *  - loopRange (TimeRange in seconds)
 *  - looping flag (whether lines are visible)
 */
class LoopRangeComponent final : public juce::Component
{
public:
    /**
     * @brief Construct a LoopRangeComponent tied to a Tracktion Edit.
     *
     * The Edit is required so we can convert from TimePosition → BeatPosition
     * using the tempoSequence.
     *
     * @param edit Reference to the Tracktion Edit.
     */
    explicit LoopRangeComponent (te::Edit& edit);

    //==============================================================================
    /** @internal Draws the loop start/end lines if looping is enabled. */
    void paint (juce::Graphics& g) override;

    //==============================================================================
    /** Set the horizontal zoom scaling (px per beat). */
    void setPixelsPerBeat (double ppb);

    /** Set the beat at the left edge of the visible region. */
    void setViewStartBeat (t::BeatPosition b);

    /** Update the loop range (TimeRange in seconds). */
    void setLoopRange (t::TimeRange range);

    /** Enable or disable drawing the loop markers. */
    void setLooping (bool shouldLoop);

private:
    //==============================================================================
    te::Edit& edit;                      ///< Reference to Edit for time ↔ beat conversion.

    double pixelsPerBeat = 100.0;        ///< Horizontal zoom factor.
    t::BeatPosition viewStartBeat { t::BeatPosition::fromBeats (0.0) };

    t::TimeRange loopRange;              ///< Loop positions in absolute time.
    bool looping = false;                ///< Whether loop lines should be drawn.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopRangeComponent)
};

