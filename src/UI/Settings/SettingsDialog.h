// Created by Claude Code on 2025-11-18.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;
class AudioSettingsPanel;
class MidiSettingsPanel;

/**
 * @brief Main settings dialog with tabbed interface for Audio and MIDI configuration.
 *
 * SettingsDialog provides a modal interface for adjusting application-wide settings.
 * It uses a tabbed layout with separate panels for different categories:
 *  - Audio: Output device, sample rate, buffer size
 *  - MIDI: Input device enable/disable
 *
 * Architecture:
 *  - Displayed via JUCE DialogWindow::launchAsync() from menu bar
 *  - Owns AudioSettingsPanel and MidiSettingsPanel
 *  - Settings changes are applied immediately (no Apply/Cancel buttons)
 *  - Uses TabbedComponent for organization
 *
 * Usage:
 *  - Launched from GrooveKitMenuBar via "File → Preferences..."
 *  - Modal dialog (blocks interaction with main window)
 *  - Changes persist via Tracktion Engine's device manager state
 */
class SettingsDialog : public juce::Component
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the settings dialog.
     *
     * @param engine Reference to AppEngine for settings access
     */
    explicit SettingsDialog (AppEngine& engine);

    /** Destructor. */
    ~SettingsDialog() override;

    //==============================================================================
    // Component Overrides

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Member Variables

    AppEngine& appEngine; ///< Reference to global engine (not owned)

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop }; ///< Tabbed container for panels

    std::unique_ptr<AudioSettingsPanel> audioPanel; ///< Owned audio settings panel
    std::unique_ptr<MidiSettingsPanel> midiPanel; ///< Owned MIDI settings panel

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsDialog)
};
