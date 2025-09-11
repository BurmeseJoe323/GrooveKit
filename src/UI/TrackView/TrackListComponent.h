#pragma once

#include "../../AppEngine/AppEngine.h"
#include "../PopupWindows/PianoRollWindow.h"
#include "PlayheadComponent.h"
#include "TrackComponent.h"
#include "TrackHeaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Represents the Component view holding tracks.
 */
class TrackListComponent final : public juce::Component
{
public:
    explicit TrackListComponent (const std::shared_ptr<AppEngine>& engine);
    ~TrackListComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void parentSizeChanged() override;
    void refreshSoloVisuals();
    void addNewTrack (int index);
    void setPixelsPerSecond (double pps);
    void setViewStart (te::TimePosition t);

private:
    const std::shared_ptr<AppEngine> appEngine;
    PlayheadComponent playhead;
    std::unique_ptr<PianoRollWindow> pianoRollWindow;
    int selectedTrackIndex;

    juce::OwnedArray<TrackComponent> tracks;
    juce::OwnedArray<TrackHeaderComponent> headers;
    juce::Array<juce::Colour> trackColors {
        juce::Colour::fromString ("#ff6b6b"),
        juce::Colour::fromString ("#f06595"),
        juce::Colour::fromString ("#cc5de8"),
        juce::Colour::fromString ("#845ef7"),
        juce::Colour::fromString ("#5c7cfa"),
        juce::Colour::fromString ("#339af0"),
        juce::Colour::fromString ("#22b8cf"),
        juce::Colour::fromString ("#20c997"),
        juce::Colour::fromString ("#51cf66"),
        juce::Colour::fromString ("#fcc419")
    };

    void updateTrackIndexes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackListComponent)
};