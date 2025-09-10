#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TrackClip final : public juce::Component
{
public:
    TrackClip();
    ~TrackClip() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void setColor (juce::Colour newColor);

private:
    juce::Colour clipColor { juce::Colours::blueviolet };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackClip)
};
