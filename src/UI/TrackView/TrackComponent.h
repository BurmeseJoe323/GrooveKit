#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackClip.h"
#include "TrackHeaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
namespace t = tracktion;

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

    /**
     * Retrieves updated clip models from app engine, clears old UIs, and draws the updated models.
     */
    void rebuildClipsFromEngine();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent& e) override; // handles right clicks

    void setPixelsPerSecond (const double pps)
    {
        pixelsPerSecond = pps;
        resized();
    }

    void setViewStart (const t::TimePosition t)
    {
        viewStart = t;
        resized();
    }

    std::function<void (int)> onRequestDeleteTrack;
    std::function<void (te::MidiClip* clip)> onRequestOpenPianoRoll;
    std::function<void (int)> onRequestOpenDrumSampler;

private:
    std::shared_ptr<AppEngine> appEngine;
    std::shared_ptr<MIDIEngine> midiEngine;
    juce::Colour trackColor;
    int trackIndex = -1;
    int numClips = 0;

    juce::OwnedArray<TrackClip> clipUIs;

    // Visual scaling
    float pixelsPerBeat = 100.0f;
    double pixelsPerSecond = 100.0; // currently unused for clip layout
    t::TimePosition viewStart = 0s;

    static int timeToX (const t::TimePosition t, const t::TimePosition view0, const double pps)
    {
        return juce::roundToInt ((t - view0).inSeconds() * pps);
    }

    static int xFromTime (const t::TimePosition t, const t::TimePosition view0, const double pps)
    {
        const double secs = (t - view0).inSeconds();
        return static_cast<int> (std::floor (secs * pps));
    }

    static int timeRangeToWidth (const t::TimeRange r, const double pps)
    {
        return juce::roundToInt (r.getLength().inSeconds() * pps);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};
