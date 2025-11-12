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
    void onInstrumentMenuRequested() override;
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

    /**
     * Updates visual state for clips to indicate which is being edited in piano roll (Written by Claude Code)
     */
    void updateClipEditedState (te::MidiClip* editedClip);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent& e) override; // handles right clicks

    void setPixelsPerBeat (const double ppb)
    {
        pixelsPerBeat = ppb;
        resized();
    }

    void setViewStartBeat (const t::BeatPosition b)
    {
        viewStartBeat = b;
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

    // Visual scaling (beat-based coordinate system)
    double pixelsPerBeat = 100.0;
    t::BeatPosition viewStartBeat = t::BeatPosition::fromBeats(0.0);

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

    // Helper to rebuild clips and restore piano roll highlight (Written by Claude Code)
    void rebuildAndRefreshHighlight();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};
