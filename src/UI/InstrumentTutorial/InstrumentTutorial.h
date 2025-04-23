#pragma once

#include "../../DatabaseManager/DatabaseManager.h"
#include "../TutorialManager/TutorialManagerComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;
class InstrumentTutorial final : public Component, public TutorialScreen
{
public:
    explicit InstrumentTutorial (DatabaseManager& dbManager);
    ~InstrumentTutorial() override = default;

    String getScreenName() const override
    {
        return "Instrument Tutorial";
    }

    Component* createContent() override
    {
        // we return a *new* instance so the manager can own it:
        return new InstrumentTutorial (db);
    }

    void paint(Graphics&) override;
    void resized() override;
    // void onFinishTutorial();
    std::function<void()> onFinishTutorial;

private:
    DatabaseManager& db;
    ComboBox waveformBox;
    Slider attackSlider, decaySlider, sustainSlider, releaseSlider, volumeSlider;
    Label attackLabel, decayLabel, sustainLabel, releaseLabel, volumeLabel;

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

    TextButton nextButton { "Next" };
    Label descriptionLabel;

    TextButton finishButton { "Finish Tutorial" };

    void advanceStep();
    void updateTutorialUI();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentTutorial)
};
