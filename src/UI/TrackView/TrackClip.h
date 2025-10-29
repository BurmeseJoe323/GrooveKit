#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;

class TrackClip final : public juce::Component
{
public:
    TrackClip();
    ~TrackClip() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void setColor (juce::Colour newColor);

    void setClip (te::Clip* newClip)        { clip = newClip;  }
    te::Clip* getClip() const               { return clip;     }

    std::function<void(te::MidiClip*)> onOpen;
    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        if (e.mods.isLeftButtonDown())
            if (e.mods.isLeftButtonDown() && onOpen)
                onOpen (dynamic_cast<te::MidiClip*>(clip));
    }

private:
    juce::Colour clipColor { juce::Colours::blueviolet };
    te::Clip* clip = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackClip)
};
