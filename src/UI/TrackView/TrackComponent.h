#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackClip.h"
#include "TrackHeaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Houses TrackHeader and TrackClip components.
 */
class TrackComponent final : public juce::Component, public TrackHeaderComponent::Listener
{
public:
    TrackComponent (const std::shared_ptr<AppEngine>& engine, int trackIndex, juce::Colour color);
    ~TrackComponent() override;

    void onInstrumentClicked() override;
    void onSettingsClicked() override;
    void onMuteToggled (bool isMuted) override;
    void onSoloToggled (bool isSolo) override;
    void onRecordArmToggled (bool isArmed) override;

    void setTrackIndex (int index);
    int getTrackIndex() const;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setPixelsPerSecond (const double pps)
    {
        pixelsPerSecond = pps;
        resized();
    }

    void setViewStart (const te::TimePosition t)
    {
        viewStart = t;
        resized();
    }

    std::function<void (int)> onRequestDeleteTrack;
    std::function<void (int)> onRequestOpenPianoRoll;
    std::function<void (int)> onRequestOpenDrumSampler;

private:
    std::shared_ptr<AppEngine> appEngine;
    juce::Colour trackColor;
    int trackIndex;
    int numClips = 0;

    std::unique_ptr<TrackClip> trackClip;

    double pixelsPerSecond = 100.0;
    te::TimePosition viewStart = 0s;

    static int timeToX (const te::TimePosition t, const te::TimePosition view0, const double pps)
    {
        return juce::roundToInt ((t - view0).inSeconds() * pps);
    }

    static int xFromTime (const te::TimePosition t, const te::TimePosition view0, const double pps)
    {
        const double secs = (t - view0).inSeconds();
        return static_cast<int> (std::floor (secs * pps));
    }

    static int timeRangeToWidth (const te::TimeRange r, const double pps)
    {
        return juce::roundToInt (r.getLength().inSeconds() * pps);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};
