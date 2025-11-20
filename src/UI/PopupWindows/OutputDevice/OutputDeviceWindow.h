#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

/**
 * @class OutputDeviceWindow
 *
 * @brief Small panel for selecting the audio output device.
 *
 * Responsibilities:
 *  - List all available output devices from AppEngine
 *  - Let the user choose one from a ComboBox
 *  - Provide a button to refresh the device list
 *  - Provide a button to revert to the system default output device
 *
 * This component is meant to be shown in a popup window or panel and
 * does not manage its own lifetime.
 */
class OutputDeviceWindow : public juce::Component,
                           private juce::ComboBox::Listener
{
public:
    /**
     * @brief Construct an OutputDeviceWindow bound to an AppEngine.
     *
     * The AppEngine is used to:
     *  - Query available output devices
     *  - Set the current output device
     *  - Revert to the system default device
     *
     * @param appEng Reference to the owning AppEngine.
     */
    explicit OutputDeviceWindow (AppEngine& appEng);

    /** Destructor. */
    ~OutputDeviceWindow() override = default;

    //==========================================================================
    /** Layouts all child components. */
    void resized() override;

private:
    //==========================================================================
    // juce::ComboBox::Listener

    /**
     * @brief Called when the user selects a different device in the ComboBox.
     *
     * Applies the chosen device to AppEngine.
     */
    void comboBoxChanged (juce::ComboBox* c) override;

    //==========================================================================
    /**
     * @brief Rebuilds the device list in the ComboBox from AppEngine.
     *
     * - Clears existing items
     * - Populates new entries from app.listOutputDevices()
     * - Sets the current selection to the active device (if any)
     */
    void refreshList();

    //==========================================================================
    AppEngine& app;                    ///< Reference to the AppEngine for device control.

    juce::Label     title   { {}, "Output device" }; ///< Static title label.
    juce::ComboBox  devices;                         ///< Drop-down list of output devices.
    juce::TextButton refreshBtn { "Refresh" };       ///< Rebuild device list.
    juce::TextButton defaultBtn { "Use System Default" }; ///< Switch back to system default.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputDeviceWindow)
};