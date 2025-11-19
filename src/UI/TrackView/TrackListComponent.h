#pragma once

#include "../../AppEngine/AppEngine.h"
#include "PlayheadComponent.h"
#include "LoopRangeComponent.h"
#include "TrackComponent.h"
#include "TrackHeaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "TimelineComponent.h"

namespace te = tracktion::engine;
namespace t = tracktion;
/**
 * Represents the Component view holding tracks.
 */
class TrackListComponent final : public juce::Component
{
public:
    explicit TrackListComponent (const std::shared_ptr<AppEngine>& engine); // should this be a reference rather than shared pointer?
    ~TrackListComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void parentSizeChanged() override;
    void refreshTrackStates() const;
    void addNewTrack (int index);

    void rebuildFromEngine();

    void armTrack(int trackIndex, bool shouldBeArmed);
    void repaintTrack(int trackIndex);

    void setPixelsPerBeat (double ppb);
    void setViewStartBeat (t::BeatPosition b);
    double getPixelsPerBeat() const
    {
        return timeline ? timeline->getPixelsPerBeat() : 100.0;
    }

    t::BeatPosition getViewStartBeat() const
    {
        return timeline ? timeline->getViewStartBeat() : t::BeatPosition::fromBeats(0.0);
    }

    bool keyPressed (const juce::KeyPress&) override;
    bool keyStateChanged (bool isKeyDown) override;
    void mouseDown (const juce::MouseEvent&) override;
    void parentHierarchyChanged() override;

private:
    const std::shared_ptr<AppEngine> appEngine;

    std::unique_ptr<ui::TimelineComponent> timeline;
    static constexpr int timelineHeight = 24;
    static constexpr int headerWidth    = 140;

    PlayheadComponent playhead;
    LoopRangeComponent loopRangeComponent;

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
    juce::TextButton loopButton { "loop" };


    void updateTrackIndexes() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackListComponent)
};