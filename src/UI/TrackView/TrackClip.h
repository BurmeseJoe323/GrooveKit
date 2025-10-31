// JUNIE
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "tracktion_graph/tracktion_graph.h"
#include <functional>

namespace te = tracktion;

class TrackClip final : public juce::Component
{
public:
    explicit TrackClip(te::MidiClip* clip, float pixelsPerBeat);
    ~TrackClip() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setColor (juce::Colour newColor);
    void setPixelsPerBeat (float ppb);

    te::MidiClip* getMidiClip() const noexcept { return clip; }

    std::function<void(te::MidiClip*)> onClicked;
    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        if (e.mods.isLeftButtonDown())
            if (e.mods.isLeftButtonDown() && onClicked)
                onClicked (clip);
    }

    std::function<void(te::MidiClip*)> onCopyRequested;
    std::function<void(te::MidiClip*)> onDuplicateRequested;
    std::function<void(te::MidiClip*, double pasteAtBeats)> onPasteRequested; // paste location in beats
    std::function<void(te::MidiClip*)> onDeleteRequested;

private:
    void updateSizeFromClip();

    te::MidiClip* clip = nullptr; // not owned
    float pixelsPerBeat = 100.0f;
    juce::Colour clipColor { juce::Colours::blueviolet };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackClip)
};
