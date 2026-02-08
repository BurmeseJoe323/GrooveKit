// Created by Claude Code on 2025-01-15.
// TransportBar - Shared transport control component for TrackEditView and MixView

#pragma once

#include "../../AppEngine/AppEngine.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace te = tracktion::engine;
namespace t = tracktion;

class AppEngine;

/**
 * Custom LookAndFeel to match toggle button font size with labels.
 * (Written by Claude Code)
 */
class TransportBarLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getToggleButtonFont(juce::ToggleButton&, int height)
    {
        juce::ignoreUnused (height);
        return juce::Font (juce::FontOptions (14.0f));
    }
};

/**
 * Transport control bar component that appears at the top of TrackEditView and MixView.
 * Contains play/pause/record buttons, BPM controls, metronome toggle, and view switcher.
 * (Written by Claude Code)
 */
class TransportBar final : public juce::Component, public juce::Label::Listener, private juce::Timer
{
public:
    enum class ViewMode
    {
        TrackEdit,  // Shows mixer icon (vertical lines) to switch to Mix view
        Mix         // Shows track list icon (horizontal lines) to switch to Track Edit view
    };

    explicit TransportBar(AppEngine& engine);
    ~TransportBar() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    /**
     * Set the current view mode to update the switch button appearance.
     * (Written by Claude Code)
     */
    void setViewMode(ViewMode mode);

    /**
     * Callback when the view switch button is clicked.
     * (Written by Claude Code)
     */
    std::function<void()> onSwitchView;

    /**
     * Update BPM display from AppEngine (called when edit is loaded).
     * (Written by Claude Code)
     */
    void updateBpmDisplay();

private:
    void setupButtons();
    void timerCallback() override;

    std::shared_ptr<AppEngine> appEngine;
    ViewMode currentViewMode = ViewMode::TrackEdit;

    // Transport controls
    juce::Label bpmLabel, bpmEditField;
    juce::ShapeButton playButton{"play", {}, {}, {}};
    juce::ShapeButton stopButton{"stop", {}, {}, {}};
    juce::ShapeButton recordButton{"record", {}, {}, {}};
    juce::ToggleButton metronomeButton{"Click"};
    juce::ShapeButton switchButton{"switch", {}, {}, {}};

    // Custom LookAndFeel for matching text sizes (Written by Claude Code)
    TransportBarLookAndFeel customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportBar)
};
