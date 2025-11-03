// JUNIE
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "tracktion_graph/tracktion_graph.h"
#include <functional>

namespace te = tracktion;

class TrackClip final : public juce::Component, private juce::ValueTree::Listener
{
public:
    explicit TrackClip(te::MidiClip* clip, float pixelsPerBeat);
    ~TrackClip() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setColor (juce::Colour newColor);
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property); // currently unused
    void setPixelsPerBeat (float ppb);

    te::MidiClip* getMidiClip() const noexcept { return clip; }
    juce::ValueTree clipState;

    std::function<void(te::MidiClip*)> onClicked;
    std::function<void(te::MidiClip*)> onCopyRequested;
    std::function<void(te::MidiClip*)> onDuplicateRequested;
    std::function<void(te::MidiClip*, double pasteAtBeats)> onPasteRequested; // paste location in beats
    std::function<void(te::MidiClip*)> onDeleteRequested;
    // Request the parent TrackComponent to show a context menu for this clip at the given beat position
    std::function<void(te::MidiClip*)> onContextMenuRequested;

    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

private:
    void updateSizeFromClip();

    te::MidiClip* clip = nullptr; // not owned
    float pixelsPerBeat = 100.0f;
    juce::Colour clipColor { juce::Colours::blueviolet };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackClip)
};
