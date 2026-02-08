#include "OutputDeviceWindow.h"
#include "AppEngine.h"

using namespace juce;

/**
 * @brief Creates the UI and wires up callbacks to AppEngine.
 *
 * - Title label ("Output device")
 * - Device ComboBox (with listener)
 * - "Refresh" button
 * - "Use System Default" button
 *
 * Immediately calls refreshList() so the ComboBox is populated.
 */
OutputDeviceWindow::OutputDeviceWindow (AppEngine& appEng)
    : app (appEng)
{
    // Title label
    addAndMakeVisible (title);
    title.setJustificationType (Justification::centredLeft);
    title.setFont (Font (FontOptions (14.0f, Font::bold)));

    // Device dropdown
    addAndMakeVisible (devices);
    devices.addListener (this);

    // Refresh button: rebuild the device list from AppEngine
    addAndMakeVisible (refreshBtn);
    refreshBtn.onClick = [this]
    {
        refreshList();
    };

    // Default button: switch to OS default and refresh UI
    addAndMakeVisible (defaultBtn);
    defaultBtn.onClick = [this]
    {
        app.setDefaultOutputDevice();
        refreshList();
    };

    // Initial population of the ComboBox
    refreshList();
}

/**
 * @brief Query AppEngine for devices and update the ComboBox.
 *
 * Behavior:
 *  - Clears all items
 *  - Adds each device as an item with ID = index + 1
 *  - If AppEngine currently has a selected device name, select it
 *  - Otherwise, select the first device (if any) without sending change events
 */
void OutputDeviceWindow::refreshList()
{
    devices.clear();

    auto outs = app.listOutputDevices();
    for (int i = 0; i < outs.size(); ++i)
        devices.addItem (outs[i], i + 1);

    auto current = app.getCurrentOutputDeviceName();
    if (current.isNotEmpty())
        devices.setText (current, dontSendNotification);
    else if (devices.getNumItems() > 0)
        devices.setSelectedItemIndex (0, dontSendNotification);
}

/**
 * @brief Lay out the title, ComboBox, and buttons vertically.
 *
 * Layout:
 *  - Title at top
 *  - Devices ComboBox
 *  - Row of [Refresh] [Use System Default] buttons
 */
void OutputDeviceWindow::resized()
{
    auto r = getLocalBounds().reduced (10);

    title.setBounds (r.removeFromTop (20));

    r.removeFromTop (8);
    devices.setBounds (r.removeFromTop (28));

    r.removeFromTop (10);
    auto row = r.removeFromTop (28);

    refreshBtn.setBounds (row.removeFromLeft (120));
    row.removeFromLeft (10);
    defaultBtn.setBounds (row.removeFromLeft (160));
}

/**
 * @brief Handle selection changes in the device ComboBox.
 *
 * When the user picks a device, forward it to AppEngine via setOutputDevice.
 */
void OutputDeviceWindow::comboBoxChanged (ComboBox* c)
{
    if (c != &devices)
        return;

    auto chosen = devices.getText();
    if (chosen.isNotEmpty())
        app.setOutputDevice (chosen);
}