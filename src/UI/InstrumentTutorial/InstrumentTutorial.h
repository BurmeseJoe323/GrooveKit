#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class InstrumentTutorial : public juce::Component
{
public:
    InstrumentTutorial();
    ~InstrumentTutorial() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    std::function<void()> onFinishTutorial;

private:
    juce::ComboBox waveformBox;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, volumeSlider;

    enum class TutorialStep
    {
        Waveform,
        Attack,
        Decay,
        Sustain,
        Release,
        Volume,
        Done
    };

    TutorialStep currentStep = TutorialStep::Waveform;

    juce::TextButton nextButton { "Next" };
    juce::Label descriptionLabel;

    juce::TextButton finishButton { "Finish Tutorial" };

    void advanceStep();
    void updateTutorialUI();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentTutorial)
};
