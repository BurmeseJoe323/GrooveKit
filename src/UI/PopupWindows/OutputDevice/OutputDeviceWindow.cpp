#include "OutputDeviceWindow.h"
#include "AppEngine.h"

using namespace juce;

OutputDeviceWindow::OutputDeviceWindow (AppEngine& appEng) : app (appEng)
{
    addAndMakeVisible (title);
    title.setJustificationType (Justification::centredLeft);
    title.setFont (Font (14.0f, Font::bold));

    addAndMakeVisible (devices);
    devices.addListener (this);

    addAndMakeVisible (refreshBtn);
    refreshBtn.onClick = [this] { refreshList(); };

    addAndMakeVisible (defaultBtn);
    defaultBtn.onClick = [this]
    {
        app.setDefaultOutputDevice();
        refreshList();
    };

    refreshList();
}

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

void OutputDeviceWindow::comboBoxChanged (ComboBox* c)
{
    if (c != &devices) return;
    auto chosen = devices.getText();
    if (chosen.isNotEmpty())
        app.setOutputDevice (chosen);
}
