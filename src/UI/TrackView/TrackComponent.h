#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackClip.h"
#include "TrackHeaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Houses TrackHeader and TrackClip components.
 */
// TrackComponent.h
class TrackComponent final : public juce::Component, public TrackHeaderComponent::Listener
{
public:
    TrackComponent (AppEngine* engine, te::AudioTrack* track, juce::Colour colour);
    ~TrackComponent() override;

    te::AudioTrack* getTrackPtr () const { return trackPtr; }

    // Derive engine index on demand from the pointer:
    int getTrackIndex () const;           // implemented in .cpp

    // UI row index (labels/selection only; never use for engine ops)
    void setUIIndex (int i) { uiIndex = i; }
    int  getUIIndex () const { return uiIndex; }

    void setPixelsPerSecond (double pps);
    double getPixelsPerSecond () const { return pixelsPerSecond; }

    void setViewStart (te::TimePosition start);
    te::TimePosition getViewStart () const { return viewStart; }

    void onInstrumentPressed (juce::Component& anchor) override;
    void onSettingsClicked() override;
    void onMuteToggled (bool isMuted) override;
    void onSoloToggled (bool isSolo) override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    std::function<void (int)> onRequestDeleteTrack;
    std::function<void (int)> onRequestOpenPianoRoll;
    std::function<void (int)> onRequestOpenDrumSampler;

private:
    AppEngine*     appEngine = nullptr;   // <<< raw pointer (you pass appEngine.get())
    te::AudioTrack* trackPtr = nullptr;   // <<< stable binding
    juce::Colour   trackColor;
    int            numClips = 0;
    int            uiIndex  = -1;

    TrackClip trackClip;

    double         pixelsPerSecond = 100.0;
    te::TimePosition viewStart = 0s;

    static int timeToX (te::TimePosition t, te::TimePosition view0, double pps)
    {
        return juce::roundToInt ((t - view0).inSeconds() * pps);
    }
    static int xFromTime (te::TimePosition t, te::TimePosition view0, double pps)
    {
        const double secs = (t - view0).inSeconds();
        return static_cast<int> (std::floor (secs * pps));
    }
    static int timeRangeToWidth (te::TimeRange r, double pps)
    {
        return juce::roundToInt (r.getLength().inSeconds() * pps);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};
