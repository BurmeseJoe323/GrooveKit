// Created by Claude Code on 2025-11-18.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

/**
 * @brief MIDI settings panel for enabling/disabling MIDI input devices.
 *
 * MidiSettingsPanel provides toggle buttons for each available MIDI input device.
 * Users can enable or disable individual devices for live MIDI input and recording.
 *
 * Architecture:
 *  - Owned by SettingsDialog (displayed in MIDI tab)
 *  - Creates toggle button for each detected MIDI device
 *  - Uses scrollable viewport for long device lists
 *  - Applies changes immediately via AudioEngine::setMidiDeviceEnabled()
 *
 * Device Detection:
 *  - Queries JUCE MidiInput::getAvailableDevices() on construction
 *  - Displays "No MIDI input devices detected" if empty
 *  - Shows device identifier and name for each device
 *
 * Usage:
 *  - Toggle buttons apply changes immediately (no Apply button)
 *  - Changes persist in Tracktion Engine's device manager state
 *  - Enabled devices will receive MIDI input when armed track is selected
 */
class MidiSettingsPanel : public juce::Component
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the MIDI settings panel.
     *
     * @param engine Reference to AppEngine for MIDI device configuration
     */
    explicit MidiSettingsPanel (AppEngine& engine);

    /** Destructor. */
    ~MidiSettingsPanel() override;

    //==============================================================================
    // Component Overrides

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Internal Methods

    /**
     * @brief Refreshes the MIDI device toggle list.
     *
     * Queries available MIDI input devices and creates toggle buttons.
     */
    void refreshDeviceList();

    /**
     * @brief Called when a MIDI device toggle is clicked.
     *
     * Applies the new enabled/disabled state via AudioEngine.
     *
     * @param deviceName Name of the MIDI device
     * @param enabled New enabled state
     */
    void onDeviceToggled (const juce::String& deviceName, bool enabled);

    //==============================================================================
    // Member Variables

    AppEngine& appEngine; ///< Reference to global engine (not owned)

    juce::Label titleLabel { {}, "MIDI Input Devices:" }; ///< Panel title
    juce::Label infoLabel { {}, "Enable or disable MIDI input devices" }; ///< Instruction text

    juce::OwnedArray<juce::ToggleButton> deviceToggles; ///< Owned toggle buttons for each device
    juce::Viewport deviceViewport; ///< Scrollable viewport for device list
    juce::Component deviceContainer; ///< Container holding toggle buttons

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiSettingsPanel)
};
