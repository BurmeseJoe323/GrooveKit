#pragma once

#include "AppEngine/AppEngine.h"
#include <juce_gui_basics/juce_gui_basics.h>


class TestComponent : public juce::Component
{
public:
    TestComponent()
    {
        startButton.setButtonText("Start MIDI");
        startButton.onClick = [this] { engine.start(); };

        addAndMakeVisible(startButton);
        setSize(400, 200);
    }

    void resized() override
    {
        startButton.setBounds(getLocalBounds().reduced(50));
    }

private:
    AppEngine engine;
    juce::TextButton startButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestComponent)
};
