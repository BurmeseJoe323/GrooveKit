#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../DatabaseManager/DatabaseManager.h"

class InstrumentTutorial : public juce::Component
{
public:
    InstrumentTutorial(DatabaseManager& dbManager);
    ~InstrumentTutorial() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    void onFinishTutorial();

private:
    DatabaseManager& db;
    juce::ComboBox waveformBox;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, volumeSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, volumeLabel;

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
