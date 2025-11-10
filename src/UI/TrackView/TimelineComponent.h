#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;
namespace t = tracktion;

namespace ui
{
    class TimelineComponent : public juce::Component
    {
    public:
        // Drop EditViewState from the ctor
        explicit TimelineComponent (te::Edit& e)
            : edit (e) {}

        // Zoom/scroll API (beat-based)
        void setPixelsPerBeat(double ppb);
        void setViewStartBeat(t::BeatPosition b);
        double getPixelsPerBeat() const { return pixelsPerBeat; }
        t::BeatPosition getViewStartBeat() const { return viewStartBeat; }

        // Optional: callback while scrubbing
        std::function<void (t::TimePosition)> onScrub;

        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;
        void mouseDrag (const juce::MouseEvent& e) override;
        void mouseUp   (const juce::MouseEvent& e) override;
        void mouseDoubleClick (const juce::MouseEvent& e) override;

        void setLoopRange (t::TimeRange r);
        t::TimeRange getLoopRange() const { return loopRange; }
        std::function<void (t::TimeRange)> onLoopRangeChanged;

        void setSnapToBeats (bool shouldSnap) { snapToBeats = shouldSnap; }
        void setEditForSnap (te::Edit* e)     { editForSnap = e; }

    private:
        te::Edit& edit;

        double pixelsPerBeat = 100.0;
        t::BeatPosition viewStartBeat { t::BeatPosition::fromBeats (0.0) };

        t::TimePosition xToTime (int x) const
        {
            // Convert x to beat position, then to time
            const auto beats = viewStartBeat.inBeats() + (double) x / pixelsPerBeat;
            const auto beatPos = t::BeatPosition::fromBeats (juce::jmax (0.0, beats));
            return edit.tempoSequence.toTime (beatPos);
        }

        void setTransportPositionFromX (int x, bool dragging);

        t::TimeRange loopRange {
            t::TimePosition::fromSeconds(0.0),
            t::TimePosition::fromSeconds(0.0)
        };
        bool hasLoop = false;

        enum class DragMode { none, dragStart, dragEnd, dragBody };
        DragMode dragMode = DragMode::none;
        double dragAnchorSec   = 0.0;
        double originalStartSec = 0.0;
        double originalEndSec   = 0.0;

        int handleWidthPx = 4;
        int hitSlopPx     = 8;

        juce::Point<int> panStartView{};
        int panStartX = 0;
        bool panning = false;

        // helpers for beat<->x in *this* timeline's coord system
        double xToBeats (int x) const
        { return viewStartBeat.inBeats() + (double) x / pixelsPerBeat; }
        int beatsToX (double beats) const
        { return (int) juce::roundToIntAccurate((beats - viewStartBeat.inBeats()) * pixelsPerBeat); }

        te::Edit* editForSnap = nullptr;
        bool snapToBeats = false;
        void snapSecondsToBeats (double& seconds) const;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineComponent)
    };
}
