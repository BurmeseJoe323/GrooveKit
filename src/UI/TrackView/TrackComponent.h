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

    // void onAddClipClicked() override;
    // // int getNumClips();
    // void onDeleteTrackClicked() override;
    // void onPianoRollClicked() override;
    void onSettingsClicked() override;
    void onMuteToggled (bool isMuted) override;
    void onSoloToggled (bool isSolo) override;
    // void onDrumSamplerClicked() override;

    void setTrackIndex (int index);
    void setEngineIndex (int i) { engineIndex = i; }
    int getEngineIndex() const { return engineIndex; }

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setPixelsPerSecond(double pps){ pixelsPerSecond = pps;  resized(); }
    void setViewStart(te::TimePosition t){ viewStart = t;          resized(); }

    std::function<void (int)> onRequestDeleteTrack;
    std::function<void (int)> onRequestOpenPianoRoll;
    std::function<void (int)> onRequestOpenDrumSampler;

private:
    std::shared_ptr<AppEngine> appEngine;
    juce::Colour trackColor;
    int trackIndex = -1;
    int engineIndex = -1;
    int numClips = 0;

    TrackClip trackClip;
    TrackHeaderComponent trackHeader;

    double pixelsPerSecond = 100.0;
    te::TimePosition viewStart = 0s;

    static int timeToX (te::TimePosition t, te::TimePosition view0, double pps)
    {
        return juce::roundToInt ((t - view0).inSeconds() * pps);
    }
    static int xFromTime (te::TimePosition t, te::TimePosition view0, double pps)
    {
        const double secs = (t - view0).inSeconds();
        return (int) std::floor (secs * pps);
    }
    static int timeRangeToWidth (te::TimeRange r, double pps)
    {
        return juce::roundToInt (r.getLength().inSeconds() * pps);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};
