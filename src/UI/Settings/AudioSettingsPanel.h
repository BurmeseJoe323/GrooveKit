// Created by Claude Code on 2025-11-18.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

/**
 * @brief Audio settings panel for output device, sample rate, and buffer size configuration.
 *
 * AudioSettingsPanel provides controls for configuring the audio playback system:
 *  - Output device selection (dropdown of available audio interfaces)
 *  - Sample rate selection (44.1kHz, 48kHz, 96kHz, etc.)
 *  - Buffer size selection (with calculated latency display)
 *  - Quick access buttons (Refresh, Use System Default)
 *
 * Architecture:
 *  - Owned by SettingsDialog (displayed in Audio tab)
 *  - Implements ComboBox::Listener for immediate settings changes
 *  - Queries AudioEngine for available devices/rates/buffers
 *  - Applies changes via AudioEngine::setOutputDeviceByName(), setBufferSize(), setSampleRate()
 *
 * Latency Calculation:
 *  - Displayed as "(latency: X.X ms)" next to buffer size
 *  - Calculated as: (bufferSize / sampleRate) * 1000.0
 *  - Updates automatically when buffer or sample rate changes
 *
 * Usage:
 *  - Changes apply immediately (no Apply button needed)
 *  - Refresh button re-enumerates devices (useful for hot-plugged interfaces)
 *  - System Default button restores OS default output device
 */
class AudioSettingsPanel : public juce::Component, private juce::ComboBox::Listener
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the audio settings panel.
     *
     * @param engine Reference to AppEngine for audio configuration
     */
    explicit AudioSettingsPanel (AppEngine& engine);

    /** Destructor. */
    ~AudioSettingsPanel() override;

    //==============================================================================
    // Component Overrides

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // ComboBox::Listener Overrides

    /**
     * @brief Called when a combo box selection changes.
     *
     * Applies the new device/rate/buffer setting via AudioEngine.
     *
     * @param combo Combo box that changed
     */
    void comboBoxChanged (juce::ComboBox* combo) override;

    //==============================================================================
    // Internal Methods

    /**
     * @brief Refreshes the output device combo box.
     *
     * Queries available audio devices and populates dropdown.
     */
    void refreshDeviceList();

    /**
     * @brief Refreshes the buffer size combo box.
     *
     * Queries available buffer sizes for current device.
     */
    void refreshBufferSizes();

    /**
     * @brief Refreshes the sample rate combo box.
     *
     * Queries available sample rates for current device.
     */
    void refreshSampleRates();

    //==============================================================================
    // Member Variables

    AppEngine& appEngine; ///< Reference to global engine (not owned)

    juce::Label deviceLabel { {}, "Output Device:" }; ///< Output device label
    juce::ComboBox deviceCombo; ///< Output device dropdown
    juce::TextButton refreshDeviceBtn { "Refresh" }; ///< Refresh device list button
    juce::TextButton defaultDeviceBtn { "Use System Default" }; ///< Reset to default device button

    juce::Label sampleRateLabel { {}, "Sample Rate:" }; ///< Sample rate label
    juce::ComboBox sampleRateCombo; ///< Sample rate dropdown

    juce::Label bufferSizeLabel { {}, "Buffer Size:" }; ///< Buffer size label
    juce::ComboBox bufferSizeCombo; ///< Buffer size dropdown
    juce::Label latencyLabel { {}, "" }; ///< Calculated latency display

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettingsPanel)
};
