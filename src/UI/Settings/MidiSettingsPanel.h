// Created by Claude Code on 2025-11-18.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

/**
 * @brief MIDI settings panel for displaying detected MIDI input devices.
 *
 * MidiSettingsPanel displays a list of all available MIDI input devices detected
 * on the system. The panel provides read-only information about which MIDI devices
 * are currently connected and available for use.
 *
 * Architecture:
 *  - Owned by SettingsDialog (displayed in MIDI tab)
 *  - Creates label for each detected MIDI device
 *  - Uses scrollable viewport for long device lists
 *  - Auto-refreshes on panel construction
 *
 * Device Detection:
 *  - Queries JUCE MidiInput::getAvailableDevices() on construction
 *  - Displays "No MIDI input devices detected" if empty
 *  - Shows device identifier and name for each device
 *
 * Usage:
 *  - Display-only panel (no user interaction required)
 *  - All detected MIDI devices are automatically available for input
 *  - MIDI input works when track is armed for recording
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
     * @brief Refreshes the MIDI device list display.
     *
     * Queries available MIDI input devices and creates labels for each.
     */
    void refreshDeviceList();

    //==============================================================================
    // Member Variables

    AppEngine& appEngine; ///< Reference to global engine (not owned)

    juce::Label titleLabel { {}, "MIDI Input Devices:" }; ///< Panel title
    juce::Label infoLabel { {}, "Detected MIDI devices" }; ///< Instruction text

    juce::OwnedArray<juce::Label> deviceLabels; ///< Owned labels for each device
    juce::Viewport deviceViewport; ///< Scrollable viewport for device list
    juce::Component deviceContainer; ///< Container holding device labels

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiSettingsPanel)
};
