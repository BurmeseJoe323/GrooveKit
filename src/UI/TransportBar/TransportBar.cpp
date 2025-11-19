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
    metronomeButton.setLookAndFeel(nullptr); // Clean up custom LookAndFeel (Written by Claude Code)
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

    // Right side: Switch button (Written by Claude Code - made smaller to match transport buttons)
    constexpr int switchButtonSize = 20;
    const auto switchArea = r.removeFromRight(switchButtonSize);
    switchButton.setBounds(switchArea.withSizeKeepingCentre(switchButtonSize, switchButtonSize));

    // Left side: BPM controls
    bpmLabel.setBounds(r.removeFromLeft(50));
    auto valueArea = r.removeFromLeft(50);
    int deltaHeight = valueArea.getHeight() / 8;
    valueArea.removeFromBottom(deltaHeight);
    valueArea.removeFromTop(deltaHeight);
    bpmEditField.setBounds(valueArea);

    // Metronome to the right of BPM field (Written by Claude Code)
    constexpr int metronomeWidth = 60;
    auto metronomeArea = r.removeFromLeft(metronomeWidth).reduced(5, 8);
    metronomeButton.setBounds(metronomeArea);

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
}

void TransportBar::setupButtons()
{
    // BPM Label (Written by Claude Code)
    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM:", juce::dontSendNotification);
    bpmLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    bpmLabel.setJustificationType(juce::Justification::right);
    bpmLabel.setFont(juce::FontOptions(14.0f));

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
    bpmEditField.setFont(juce::FontOptions(14.0f));

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
    metronomeButton.setLookAndFeel(&customLookAndFeel); // Match text size with BPM label
    metronomeButton.onClick = [this] {
        appEngine->setClickTrackEnabled(metronomeButton.getToggleState());
    };

    // View Switch Button - initialize with mixer icon (vertical lines)
    juce::Path mixerIcon;
    constexpr float lineWidth = 0.08f;
    constexpr float lineHeight = 0.6f;
    constexpr float startY = (1.0f - lineHeight) / 2.0f;
    constexpr float cornerRadius = 0.02f;

    mixerIcon.addRoundedRectangle(0.25f - lineWidth/2, startY, lineWidth, lineHeight, cornerRadius);
    mixerIcon.addRoundedRectangle(0.50f - lineWidth/2, startY, lineWidth, lineHeight, cornerRadius);
    mixerIcon.addRoundedRectangle(0.75f - lineWidth/2, startY, lineWidth, lineHeight, cornerRadius);

    switchButton.setShape(mixerIcon, false, true, false);
    switchButton.setColours(juce::Colours::lightgrey, juce::Colours::white, juce::Colours::darkgrey);
    switchButton.onClick = [this] {
        if (onSwitchView)
            onSwitchView();
    };
    addAndMakeVisible(switchButton);
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

    // Update switch button icon to show destination view
    juce::Path icon;
    constexpr float lineThickness = 0.08f;
    constexpr float lineLength = 0.6f;
    constexpr float cornerRadius = 0.02f;

    switch (currentViewMode)
    {
        case ViewMode::TrackEdit:
        {
            // Show mixer icon (vertical lines) - switches to Mix view
            const float startY = (1.0f - lineLength) / 2.0f;
            icon.addRoundedRectangle(0.25f - lineThickness/2, startY, lineThickness, lineLength, cornerRadius);
            icon.addRoundedRectangle(0.50f - lineThickness/2, startY, lineThickness, lineLength, cornerRadius);
            icon.addRoundedRectangle(0.75f - lineThickness/2, startY, lineThickness, lineLength, cornerRadius);
            break;
        }
        case ViewMode::Mix:
        {
            // Show track list icon (horizontal lines) - switches to Track Edit view
            const float startX = (1.0f - lineLength) / 2.0f;
            icon.addRoundedRectangle(startX, 0.25f - lineThickness/2, lineLength, lineThickness, cornerRadius);
            icon.addRoundedRectangle(startX, 0.50f - lineThickness/2, lineLength, lineThickness, cornerRadius);
            icon.addRoundedRectangle(startX, 0.75f - lineThickness/2, lineLength, lineThickness, cornerRadius);
            break;
        }
    }

    switchButton.setShape(icon, false, true, false);
    switchButton.setVisible(true);
    switchButton.repaint();
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
        // Make record button brighter when recording for visual feedback
        recordButton.setColours (juce::Colours::red.brighter (0.3f),
                                 juce::Colours::lightcoral.brighter (0.3f),
                                 juce::Colours::red.brighter (0.5f));
    }
    else
    {
        // Grey normally, dark red on hover hint, darker grey on press
        recordButton.setColours (juce::Colours::lightgrey,
                                 juce::Colours::darkred,
                                 juce::Colours::darkgrey);
    }
}
