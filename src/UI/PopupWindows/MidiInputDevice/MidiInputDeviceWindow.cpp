#include "MidiInputDeviceWindow.h"
#include "AppEngine.h"

using namespace juce;

MidiInputDeviceWindow::MidiInputDeviceWindow (AppEngine& appEng) : app (appEng)
{
    addAndMakeVisible (title);
    title.setJustificationType (Justification::centredLeft);
    title.setFont (Font (FontOptions (14.0f, Font::bold)));

    addAndMakeVisible (devices);
    devices.addListener (this);

    addAndMakeVisible (refreshBtn);
    refreshBtn.onClick = [this] { refreshList(); };

    addAndMakeVisible (disableBtn);
    disableBtn.onClick = [this]
    {
        app.disconnectAllMidiInputs();
        refreshList();
    };

    refreshList();
}

void MidiInputDeviceWindow::refreshList()
{
    devices.clear();

    auto inputs = app.listMidiInputDevices();
    for (int i = 0; i < inputs.size(); ++i)
        devices.addItem (inputs[i], i + 1);

    auto current = app.getCurrentMidiInputDeviceName();
    if (current.isNotEmpty())
        devices.setText (current, dontSendNotification);
    else if (devices.getNumItems() > 0)
        devices.setText ("(No device selected)", dontSendNotification);
}

void MidiInputDeviceWindow::resized()
{
    auto r = getLocalBounds().reduced (10);
    title.setBounds (r.removeFromTop (20));

    r.removeFromTop (8);
    devices.setBounds (r.removeFromTop (28));

    r.removeFromTop (10);
    auto row = r.removeFromTop (28);
    refreshBtn.setBounds (row.removeFromLeft (120));
    row.removeFromLeft (10);
    disableBtn.setBounds (row.removeFromLeft (160));
}

void MidiInputDeviceWindow::comboBoxChanged (ComboBox* c)
{
    if (c != &devices) return;
    auto chosen = devices.getText();
    if (chosen.isNotEmpty() && chosen != "(No device selected)")
        app.setMidiInputDevice (chosen);
}
