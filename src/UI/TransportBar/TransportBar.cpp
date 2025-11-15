// Created by Claude Code on 2025-01-15.
// TransportBar - Shared transport control component implementation

#include "TransportBar.h"
#include "../../AppEngine/ValidationUtils.h"

TransportBar::TransportBar(AppEngine& engine)
{
    appEngine = std::shared_ptr<AppEngine>(&engine, [](AppEngine*) {});

    setupButtons();

    // Start timer for record button animation
    startTimer(100);
}

TransportBar::~TransportBar()
{
    stopTimer();
}

void TransportBar::paint(juce::Graphics& g)
{
    // Dark background for entire transport bar
    g.setColour(juce::Colour(0xFF212529));
    g.fillAll();

    // Bottom border
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.drawHorizontalLine(getHeight() - 1, 0.0f, static_cast<float>(getWidth()));
}

void TransportBar::resized()
{
    auto r = getLocalBounds().reduced(10, 0);

    // Right side: Switch button
    const auto switchArea = r.removeFromRight(50);
    switchButton.setBounds(switchArea);

    // Left side: BPM controls
    bpmLabel.setBounds(r.removeFromLeft(50));
    auto valueArea = r.removeFromLeft(50);
    int deltaHeight = valueArea.getHeight() / 8;
    valueArea.removeFromBottom(deltaHeight);
    valueArea.removeFromTop(deltaHeight);
    bpmEditField.setBounds(valueArea);

    // Center: Transport buttons
    constexpr int buttonSize = 20;
    constexpr int buttonGap = 10;
    constexpr int transportWidth = (buttonSize * 3) + (buttonGap * 2);
    auto transportBounds = r.withSizeKeepingCentre(transportWidth, buttonSize);
    stopButton.setBounds(transportBounds.removeFromLeft(buttonSize));
    transportBounds.removeFromLeft(buttonGap);
    playButton.setBounds(transportBounds.removeFromLeft(buttonSize));
    transportBounds.removeFromLeft(buttonGap);
    recordButton.setBounds(transportBounds.removeFromLeft(buttonSize));

    // Metronome button to the right of transport controls
    constexpr int metronomeWidth = 60;
    auto metronomeArea = r.removeFromRight(metronomeWidth).reduced(5, 8);
    metronomeButton.setBounds(metronomeArea);
}

void TransportBar::setupButtons()
{
    // BPM Label (Written by Claude Code)
    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM:", juce::dontSendNotification);
    bpmLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    bpmLabel.setJustificationType(juce::Justification::right);

    // BPM Edit Field (Written by Claude Code)
    addAndMakeVisible(bpmEditField);
    bpmEditField.setText(juce::String(appEngine->getBpm()), juce::dontSendNotification);
    bpmEditField.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    bpmEditField.setColour(juce::Label::outlineColourId, juce::Colours::lightgrey.brighter(0.5f));
    bpmEditField.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey.darker());
    bpmEditField.setJustificationType(juce::Justification::centred);
    bpmEditField.setEditable(true);
    bpmEditField.addListener(this);
    bpmEditField.setMouseCursor(juce::MouseCursor::IBeamCursor);

    // Stop Button (Written by Claude Code)
    juce::Path stopShape;
    stopShape.addRectangle(0.0f, 0.0f, 1.0f, 1.0f);
    stopButton.setShape(stopShape, true, true, false);
    stopButton.setColours(juce::Colours::lightgrey, juce::Colours::white, juce::Colours::darkgrey);
    stopButton.onClick = [this] { appEngine->stop(); };
    addAndMakeVisible(stopButton);

    // Play Button (Written by Claude Code)
    juce::Path playShape;
    playShape.addTriangle(0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f);
    playButton.setShape(playShape, true, true, false);
    playButton.setColours(juce::Colours::lightgrey, juce::Colours::white, juce::Colours::darkgrey);
    playButton.onClick = [this] { appEngine->play(); };
    addAndMakeVisible(playButton);

    // Record Button (Written by Claude Code)
    juce::Path recordShape;
    recordShape.addEllipse(0.0f, 0.0f, 1.0f, 1.0f);
    recordButton.setShape(recordShape, true, true, false);
    recordButton.setColours(juce::Colours::red, juce::Colours::lightcoral, juce::Colours::maroon);
    recordButton.onClick = [this] { appEngine->toggleRecord(); };
    addAndMakeVisible(recordButton);

    // Metronome Toggle (Written by Claude Code)
    addAndMakeVisible(metronomeButton);
    metronomeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::lightgrey);
    metronomeButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::lightgreen);
    metronomeButton.setToggleState(appEngine->isClickTrackEnabled(), juce::dontSendNotification);
    metronomeButton.onClick = [this] {
        appEngine->setClickTrackEnabled(metronomeButton.getToggleState());
    };

    // View Switch Button (Written by Claude Code)
    addAndMakeVisible(switchButton);
    switchButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0x00000000));
    switchButton.setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    switchButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    switchButton.onClick = [this] {
        if (onSwitchView)
            onSwitchView();
    };
}

void TransportBar::labelTextChanged(juce::Label* labelThatHasChanged)
{
    // Handle BPM change (Written by Claude Code)
    if (labelThatHasChanged == &bpmEditField)
    {
        const std::string text = bpmEditField.getText().toStdString();

        if (!ValidationUtils::isValidNumeric(text))
        {
            bpmEditField.setText(juce::String(appEngine->getBpm()), juce::dontSendNotification);
            return;
        }

        double newBpm = ValidationUtils::constrainAndRoundBpm(std::stod(text));

        appEngine->setBpm(newBpm);
        bpmEditField.setText(juce::String(newBpm, 2), juce::dontSendNotification);
    }
}

void TransportBar::setViewMode(ViewMode mode)
{
    currentViewMode = mode;

    // Update switch button text based on current view (Written by Claude Code)
    switch (currentViewMode)
    {
        case ViewMode::TrackEdit:
            switchButton.setButtonText("|||");  // Vertical lines - switches to Mix
            break;
        case ViewMode::Mix:
            switchButton.setButtonText("≡");    // Horizontal lines - switches to Track
            break;
    }
}

void TransportBar::updateBpmDisplay()
{
    // Update BPM field from AppEngine (called when edit is loaded) (Written by Claude Code)
    bpmEditField.setText(juce::String(appEngine->getBpm()), juce::dontSendNotification);
}

void TransportBar::timerCallback()
{
    // Update record button appearance based on recording state (Written by Claude Code)
    const bool isRecording = appEngine->isRecording();

    if (isRecording)
    {
        // Make record button brighter when recording
        recordButton.setColours(juce::Colours::red.brighter(0.3f),
                                juce::Colours::lightcoral.brighter(0.3f),
                                juce::Colours::red.brighter(0.5f));
    }
    else
    {
        // Normal colors when not recording
        recordButton.setColours(juce::Colours::red,
                                juce::Colours::lightcoral,
                                juce::Colours::maroon);
    }
}
