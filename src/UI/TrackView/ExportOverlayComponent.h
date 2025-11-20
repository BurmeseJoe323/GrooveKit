#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @class ExportOverlayComponent
 *
 * @brief Full-screen overlay displayed during audio export.
 *
 * This semi-transparent modal overlay blocks all interaction with the UI
 * while rendering is in progress. It includes:
 *   - A centered title: "Exporting audio..."
 *   - A short message explaining that rendering is happening
 *   - An indeterminate ProgressBar animating until export completes
 *
 * The overlay is meant to be added on top of the MainComponent (or editor)
 * while Tracktion Engine performs the render. Once finished, simply remove
 * it from the parent component.
 */
class ExportOverlayComponent : public juce::Component
{
public:
    /**
     * @brief Constructor sets up labels, progress bar, and mouse interception.
     *
     * progressValue is set to -1.0, which tells juce::ProgressBar
     * to use its built-in "indeterminate" animation style (a sweeping bar).
     */
    ExportOverlayComponent()
        : progressValue (-1.0),   // -1 = indeterminate animation
          progressBar (progressValue)
    {
        setInterceptsMouseClicks (true, true); // Block interactions with components underneath

        // ---- Title Label ----
        addAndMakeVisible (titleLabel);
        titleLabel.setText ("Exporting audio...", juce::dontSendNotification);
        titleLabel.setJustificationType (juce::Justification::centred);
        titleLabel.setFont (juce::Font (juce::FontOptions (20.0f, juce::Font::bold)));

        // ---- Message Label ----
        addAndMakeVisible (messageLabel);
        messageLabel.setText ("Please wait while GrooveKit renders your project.",
                              juce::dontSendNotification);
        messageLabel.setJustificationType (juce::Justification::centred);

        // ---- Progress Bar ----
        addAndMakeVisible (progressBar);
        progressBar.setPercentageDisplay (false); // Only show the animated bar
    }

    //==============================================================================
    /**
     * @brief Layout the centered title, message, and progress bar.
     *
     * A large central rectangle is used to position the UI. The overlay is
     * intended to resize gracefully with the window.
     */
    void resized() override
    {
        auto bounds = getLocalBounds();

        // Central panel area
        auto centreBox = bounds.reduced (bounds.getWidth() / 4,
                                         bounds.getHeight() / 3);

        titleLabel.setBounds   (centreBox.removeFromTop (40));
        centreBox.removeFromTop (10);
        messageLabel.setBounds (centreBox.removeFromTop (40));
        centreBox.removeFromTop (20);
        progressBar.setBounds  (centreBox.removeFromTop (30));
    }

    /**
     * @brief Draws the darkened background and centered rounded overlay box.
     */
    void paint (juce::Graphics& g) override
    {
        // Dim the whole screen
        g.fillAll (juce::Colours::black.withAlpha (0.6f));

        auto bounds = getLocalBounds();
        auto centreBox = bounds.reduced (bounds.getWidth() / 4,
                                         bounds.getHeight() / 3);

        // Main panel background
        g.setColour (juce::Colours::black.withAlpha (0.8f));
        g.fillRoundedRectangle (centreBox.toFloat(), 8.0f);

        // Subtle outline
        g.setColour (juce::Colours::white.withAlpha (0.2f));
        g.drawRoundedRectangle (centreBox.toFloat(), 8.0f, 1.0f);
    }

private:
    //==============================================================================
    double progressValue;              ///< -1.0 = JUCE indeterminate progress mode
    juce::ProgressBar progressBar;     ///< Animated progress indicator
    juce::Label titleLabel, messageLabel;
};