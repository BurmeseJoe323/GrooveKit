#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ExportOverlayComponent : public juce::Component
{
public:
    ExportOverlayComponent()
        : progressValue ( -1.0 ), // -1 = indeterminate animation
          progressBar (progressValue)
    {
        setInterceptsMouseClicks (true, true); // block clicks to stuff underneath

        addAndMakeVisible (titleLabel);
        titleLabel.setText ("Exporting audio...", juce::dontSendNotification);
        titleLabel.setJustificationType (juce::Justification::centred);
        titleLabel.setFont (juce::Font (juce::FontOptions (20.0f, juce::Font::bold)));

        addAndMakeVisible (messageLabel);
        messageLabel.setText ("Please wait while GrooveKit renders your project.",
                              juce::dontSendNotification);
        messageLabel.setJustificationType (juce::Justification::centred);

        addAndMakeVisible (progressBar);
        progressBar.setPercentageDisplay (false); // just an animated bar
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        // Big dim rectangle in the middle
        auto centreBox = bounds.reduced (bounds.getWidth() / 4,
                                         bounds.getHeight() / 3);

        titleLabel.setBounds   (centreBox.removeFromTop (40));
        centreBox.removeFromTop (10);
        messageLabel.setBounds (centreBox.removeFromTop (40));
        centreBox.removeFromTop (20);
        progressBar.setBounds  (centreBox.removeFromTop (30));
    }

    void paint (juce::Graphics& g) override
    {
        // Dim whole background
        g.fillAll (juce::Colours::black.withAlpha (0.6f));

        auto bounds = getLocalBounds();
        auto centreBox = bounds.reduced (bounds.getWidth() / 4,
                                         bounds.getHeight() / 3);

        g.setColour (juce::Colours::black.withAlpha (0.8f));
        g.fillRoundedRectangle (centreBox.toFloat(), 8.0f);

        g.setColour (juce::Colours::white.withAlpha (0.2f));
        g.drawRoundedRectangle (centreBox.toFloat(), 8.0f, 1.0f);
    }

private:
    double progressValue;             // stays -1.0 for indeterminate animation
    juce::ProgressBar progressBar;
    juce::Label titleLabel, messageLabel;
};

