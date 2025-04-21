#include "InstrumentTutorial.h"

InstrumentTutorial::InstrumentTutorial(DatabaseManager& dbManager)
    : db(dbManager)
{
    waveformBox.addItemList({ "Sine", "Square", "Triangle", "Saw" }, 1);
    waveformBox.setSelectedId(1);
    addAndMakeVisible(waveformBox);

    auto setupSlider = [this](juce::Slider& s, const juce::String& name, double min, double max) {
        s.setRange(min, max);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        s.setSliderStyle(juce::Slider::LinearVertical);
        s.setName(name);
        addAndMakeVisible(s);
    };

    setupSlider(attackSlider, "Attack", 0.0, 5.0);
    setupSlider(decaySlider, "Decay", 0.0, 5.0);
    setupSlider(sustainSlider, "Sustain", 0.0, 1.0);
    setupSlider(releaseSlider, "Release", 0.0, 5.0);
    setupSlider(volumeSlider, "Volume", 0.0, 1.0);
    volumeSlider.setValue(0.8);

    auto setupLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label);
    };

    setupLabel(attackLabel, "Attack");
    setupLabel(decayLabel, "Decay");
    setupLabel(sustainLabel, "Sustain");
    setupLabel(releaseLabel, "Release");
    setupLabel(volumeLabel, "Volume");

    addAndMakeVisible(descriptionLabel);
    descriptionLabel.setJustificationType(juce::Justification::centred);
    descriptionLabel.setFont(16.0f);
    descriptionLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(nextButton);
    nextButton.setButtonText("Next");
    nextButton.onClick = [this]() { advanceStep(); };

    addAndMakeVisible(finishButton);
    finishButton.setButtonText("Finish Tutorial");
    finishButton.onClick = [this]() {
        if (onFinishTutorial){
            db.addCompletedTutorial("InstrumentTutorial", "User1");
            onFinishTutorial();
        }
    };

    updateTutorialUI();
}

void InstrumentTutorial::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("Instrument Tutorial", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
}

void InstrumentTutorial::resized()
{
    auto area = getLocalBounds().reduced(10);

    descriptionLabel.setBounds(area.removeFromTop(70));
    waveformBox.setBounds(area.removeFromTop(40).withSizeKeepingCentre(300, 30));
    area.removeFromTop(10);

    auto sliderRow = area.removeFromTop(140);

    int numVisible = 0;
    if (attackSlider.isVisible())   numVisible++;
    if (decaySlider.isVisible())    numVisible++;
    if (sustainSlider.isVisible())  numVisible++;
    if (releaseSlider.isVisible())  numVisible++;
    if (volumeSlider.isVisible())   numVisible++;

    const int sliderWidth = numVisible > 0 ? sliderRow.getWidth() / numVisible : 0;
    int labelHeight = 20;
    int sliderHeight = sliderRow.getHeight() - labelHeight;
    int labelY = sliderRow.getBottom() - labelHeight;
    int x = sliderRow.getX();
    if (attackSlider.isVisible()) {
        attackSlider.setBounds(x, sliderRow.getY(), sliderWidth, sliderHeight);
        attackLabel.setBounds(x, labelY, sliderWidth, labelHeight);
        x += sliderWidth;
    }
    if (decaySlider.isVisible()) {
        decaySlider.setBounds(x, sliderRow.getY(), sliderWidth, sliderHeight);
        decayLabel.setBounds(x, labelY, sliderWidth, labelHeight);
        x += sliderWidth;
    }
    if (sustainSlider.isVisible()) {
        sustainSlider.setBounds(x, sliderRow.getY(), sliderWidth, sliderHeight);
        sustainLabel.setBounds(x, labelY, sliderWidth, labelHeight);
        x += sliderWidth;
    }
    if (releaseSlider.isVisible()) {
        releaseSlider.setBounds(x, sliderRow.getY(), sliderWidth, sliderHeight);
        releaseLabel.setBounds(x, labelY, sliderWidth, labelHeight);
        x += sliderWidth;
    }
    if (volumeSlider.isVisible()) {
        volumeSlider.setBounds(x, sliderRow.getY(), sliderWidth, sliderHeight);
        volumeLabel.setBounds(x, labelY, sliderWidth, labelHeight);
    }

    auto footer = area.removeFromBottom(50);
    nextButton.setBounds(footer.withSizeKeepingCentre(120, 30));
    finishButton.setBounds(footer.withSizeKeepingCentre(160, 30));
}

void InstrumentTutorial::advanceStep()
{
    switch (currentStep)
    {
        case TutorialStep::Waveform: currentStep = TutorialStep::Attack; break;
        case TutorialStep::Attack:   currentStep = TutorialStep::Decay; break;
        case TutorialStep::Decay:    currentStep = TutorialStep::Sustain; break;
        case TutorialStep::Sustain:  currentStep = TutorialStep::Release; break;
        case TutorialStep::Release:  currentStep = TutorialStep::Volume; break;
        case TutorialStep::Volume:   currentStep = TutorialStep::Done; break;
        case TutorialStep::Done:     break;
    }

    updateTutorialUI();
}

void InstrumentTutorial::updateTutorialUI()
{
    waveformBox.setVisible(true);
    attackSlider.setVisible(currentStep >= TutorialStep::Attack);
    decaySlider.setVisible(currentStep >= TutorialStep::Decay);
    sustainSlider.setVisible(currentStep >= TutorialStep::Sustain);
    releaseSlider.setVisible(currentStep >= TutorialStep::Release);
    volumeSlider.setVisible(currentStep >= TutorialStep::Volume);

    attackLabel.setVisible(currentStep >= TutorialStep::Attack);
    decayLabel.setVisible(currentStep >= TutorialStep::Decay);
    sustainLabel.setVisible(currentStep >= TutorialStep::Sustain);
    releaseLabel.setVisible(currentStep >= TutorialStep::Release);
    volumeLabel.setVisible(currentStep >= TutorialStep::Volume);

    nextButton.setVisible(currentStep != TutorialStep::Done);
    finishButton.setVisible(currentStep == TutorialStep::Done);

    switch (currentStep)
    {
        case TutorialStep::Waveform:
            descriptionLabel.setText("This selects the waveform type for the synth. \nTry it out and click next when you're ready to move on.", juce::dontSendNotification);
            break;
        case TutorialStep::Attack:
            descriptionLabel.setText("Attack controls how quickly the note fades in. \nTry it out and click next when you're ready to move on.", juce::dontSendNotification);
            break;
        case TutorialStep::Decay:
            descriptionLabel.setText("Decay controls how quickly the volume drops after the attack. \nTry it out and click next when you're ready to move on.", juce::dontSendNotification);
            break;
        case TutorialStep::Sustain:
            descriptionLabel.setText("Sustain sets the level held while the note is held down. \nTry it out and click next when you're ready to move on.", juce::dontSendNotification);
            break;
        case TutorialStep::Release:
            descriptionLabel.setText("Release controls how long the note fades out after letting go. \nTry it out and click next when you're ready to move on.", juce::dontSendNotification);
            break;
        case TutorialStep::Volume:
            descriptionLabel.setText("Volume adjusts the overall loudness of the synth. \nTry it out and click next when you're ready to move on.", juce::dontSendNotification);
            break;
        case TutorialStep::Done:
            descriptionLabel.setText("You've completed the instrument tutorial! \nClick finish to move on to the next tutorial", juce::dontSendNotification);
            break;
    }

    resized();
}

