#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

namespace ui
{
    class TimelineComponent : public juce::Component
    {
    public:
        // Drop EditViewState from the ctor
        explicit TimelineComponent (te::Edit& e)
            : edit (e) {}

        // Zoom/scroll API
        void setPixelsPerSecond(double pps);
        void setViewStart(te::TimePosition t);
        double getPixelsPerSecond() const { return pixelsPerSecond; }
        te::TimePosition getViewStart() const { return viewStart; }

        // Optional: callback while scrubbing
        std::function<void (te::TimePosition)> onScrub;

        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;
        void mouseDrag (const juce::MouseEvent& e) override;
        void mouseUp   (const juce::MouseEvent& e) override;
        void mouseDoubleClick (const juce::MouseEvent& e) override;

        void setLoopRange (te::TimeRange r);
        te::TimeRange getLoopRange() const { return loopRange; }
        std::function<void (te::TimeRange)> onLoopRangeChanged;

        void setSnapToBeats (bool shouldSnap) { snapToBeats = shouldSnap; }
        void setEditForSnap (te::Edit* e)     { editForSnap = e; }

    private:
        te::Edit& edit;

        double pixelsPerSecond = 100.0;
        te::TimePosition viewStart { te::TimePosition::fromSeconds (0.0) };

        te::TimePosition xToTime (int x) const
        {
            const auto secs = viewStart.inSeconds() + (double) x / pixelsPerSecond;
            return te::TimePosition::fromSeconds (juce::jmax (0.0, secs));
        }

        void setTransportPositionFromX (int x, bool dragging);

        te::TimeRange loopRange {
            te::TimePosition::fromSeconds(0.0),
            te::TimePosition::fromSeconds(0.0)
        };
        bool hasLoop = false;

        enum class DragMode { none, dragStart, dragEnd, dragBody };
        DragMode dragMode = DragMode::none;
        double dragAnchorSec   = 0.0;
        double originalStartSec = 0.0;
        double originalEndSec   = 0.0;

        int handleWidthPx = 6;
        int hitSlopPx     = 8;

        juce::Point<int> panStartView{};
        int panStartX = 0;
        bool panning = false;

        // helpers for time<->x in *this* timeline’s coord system
        double xToTimeSec (int x) const
        { return viewStart.inSeconds() + (double) x / pixelsPerSecond; }
        int timeSecToX (double t) const
        { return (int) juce::roundToIntAccurate((t - viewStart.inSeconds()) * pixelsPerSecond); }

        te::Edit* editForSnap = nullptr;
        bool snapToBeats = false;
        void snapSecondsToBeats (double& seconds) const;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineComponent)
    };
}
