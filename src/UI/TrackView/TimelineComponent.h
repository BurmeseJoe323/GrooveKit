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

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimelineComponent)
    };
}
