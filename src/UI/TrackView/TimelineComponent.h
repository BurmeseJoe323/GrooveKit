#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;
namespace t  = tracktion;

/**
 * @namespace ui
 * @brief Namespace for UI components specific to GrooveKit.
 */
namespace ui
{
    /**
     * @class TimelineComponent
     *
     * @brief Beat-based timeline at the top of the Edit view.
     *
     * Responsibilities:
     *  - Draw a beat/bar ruler using Tracktion's tempo sequence
     *  - Provide interactive loop range selection and dragging
     *  - Support horizontal panning via middle-mouse / shift+drag
     *  - Optionally snap loop operations to whole beats
     *  - Optionally notify listeners while scrubbing (`onScrub`)
     *
     * The component works in **beats** for positioning and converts to
     * Tracktion `TimePosition` using the `Edit`'s `tempoSequence`.
     */
    class TimelineComponent : public juce::Component
    {
    public:
        /**
         * @brief Constructs a TimelineComponent bound to a Tracktion Edit.
         *
         * The Edit is used for time/beat conversions and for transport control.
         *
         * @param e Reference to the owning Tracktion Edit.
         */
        explicit TimelineComponent (te::Edit& e)
            : edit (e) {}

        //==========================================================================
        // Zoom/scroll API (beat-based)

        /**
         * @brief Sets how many pixels correspond to one beat on the timeline.
         *
         * Values are clamped in the implementation to avoid non-sensical zoom.
         *
         * @param ppb New pixels-per-beat scale factor.
         */
        void setPixelsPerBeat (double ppb);

        /**
         * @brief Sets the beat at the left edge of the visible view.
         *
         * @param b Beat position where the visible region starts.
         */
        void setViewStartBeat (t::BeatPosition b);

        /** @return Current pixels-per-beat scale factor. */
        double getPixelsPerBeat() const { return pixelsPerBeat; }

        /** @return Current view start beat. */
        t::BeatPosition getViewStartBeat() const { return viewStartBeat; }

        //==========================================================================
        /**
         * @brief Optional callback invoked while scrubbing on the timeline.
         *
         * The callback receives the current transport time position.
         */
        std::function<void (t::TimePosition)> onScrub;

        //==========================================================================
        // juce::Component overrides

        void paint  (juce::Graphics& g) override;
        void mouseDown        (const juce::MouseEvent& e) override;
        void mouseDrag        (const juce::MouseEvent& e) override;
        void mouseUp          (const juce::MouseEvent& e) override;
        void mouseDoubleClick (const juce::MouseEvent& e) override;

        //==========================================================================
        // Loop range API

        /**
         * @brief Programmatically set the loop range displayed on the timeline.
         *
         * Passing a zero-length range clears the visual loop (hasLoop becomes false).
         */
        void setLoopRange (t::TimeRange r);

        /** @return Current loop range in time coordinates. */
        t::TimeRange getLoopRange() const { return loopRange; }

        /**
         * @brief Callback invoked whenever the loop range changes via user interaction
         *        or by calling setLoopRange().
         */
        std::function<void (t::TimeRange)> onLoopRangeChanged;

        //==========================================================================
        // Snap configuration

        /**
         * @brief Enable or disable snapping loop edits to whole beats.
         *
         * @param shouldSnap True to snap, false for continuous positioning.
         */
        void setSnapToBeats (bool shouldSnap) { snapToBeats = shouldSnap; }

        /**
         * @brief Set the Edit used for snapping computations.
         *
         * Typically this is the same Edit as the constructor, but is kept
         * separate in case snapping needs a different context.
         */
        void setEditForSnap (te::Edit* e) { editForSnap = e; }

    private:
        //==========================================================================
        // Core state

        te::Edit& edit;  ///< Owning Edit used for tempo mapping and transport.

        double pixelsPerBeat = 100.0; ///< Horizontal zoom (px per beat).
        t::BeatPosition viewStartBeat { t::BeatPosition::fromBeats (0.0) };

        /**
         * @brief Convert an x-coordinate in this component to a TimePosition
         *        using the Edit's tempoSequence.
         */
        t::TimePosition xToTime (int x) const
        {
            // Convert x to beat position, then to time
            const auto beats   = viewStartBeat.inBeats() + (double) x / pixelsPerBeat;
            const auto beatPos = t::BeatPosition::fromBeats (juce::jmax (0.0, beats));
            return edit.tempoSequence.toTime (beatPos);
        }

        /** Update the Edit's transport based on a mouse x coordinate. */
        void setTransportPositionFromX (int x, bool dragging);

        //==========================================================================
        // Loop range

        t::TimeRange loopRange {
            t::TimePosition::fromSeconds (0.0),
            t::TimePosition::fromSeconds (0.0)
        };

        bool hasLoop = false;

        enum class DragMode
        {
            none,
            dragStart,
            dragEnd,
            dragBody
        };

        DragMode dragMode        = DragMode::none;
        double   dragAnchorSec   = 0.0;
        double   originalStartSec = 0.0;
        double   originalEndSec   = 0.0;

        int handleWidthPx = 4;  ///< Width of loop handle grab zones.
        int hitSlopPx     = 8;  ///< Pixel tolerance for handle hit testing.

        //==========================================================================
        // Panning state (middle mouse or Shift+drag)

        juce::Point<int> panStartView {};
        int  panStartX = 0;
        bool panning   = false;

        //==========================================================================
        // Helpers for beat <-> x conversions in this component's coordinate system

        /** @return Beat position corresponding to pixel x. */
        double xToBeats (int x) const
        {
            return viewStartBeat.inBeats() + (double) x / pixelsPerBeat;
        }

        /** @return Pixel x corresponding to the given beat position. */
        int beatsToX (double beats) const
        {
            return (int) juce::roundToIntAccurate ((beats - viewStartBeat.inBeats()) * pixelsPerBeat);
        }

        //==========================================================================
        // Snapping

        te::Edit* editForSnap = nullptr; ///< Edit to use when snapping seconds to beats.
        bool      snapToBeats = false;   ///< Whether snapping is enabled.

        /**
         * @brief Snap a time value in seconds to the nearest beat using editForSnap.
         *
         * Does nothing if snapping is disabled or editForSnap is null.
         */
        void snapSecondsToBeats (double& seconds) const;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineComponent)
    };
}