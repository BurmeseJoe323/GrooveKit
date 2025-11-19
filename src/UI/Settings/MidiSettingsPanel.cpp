// Created by Claude Code on 2025-11-18.
#include "MidiSettingsPanel.h"
#include "../../AppEngine/AppEngine.h"

using namespace juce;

MidiSettingsPanel::MidiSettingsPanel (AppEngine& engine)
    : appEngine (engine)
{
    addAndMakeVisible (titleLabel);
    titleLabel.setJustificationType (Justification::centredLeft);
    titleLabel.setFont (Font (14.0f, Font::bold));

    addAndMakeVisible (infoLabel);
    infoLabel.setJustificationType (Justification::centredLeft);
    infoLabel.setFont (Font (13.0f));
    infoLabel.setColour (Label::textColourId, Colours::lightgrey);

    addAndMakeVisible (deviceViewport);
    deviceViewport.setViewedComponent (&deviceContainer, false);
    deviceViewport.setScrollBarsShown (true, false);

    deviceContainer.setSize (400, 400); // Will be resized based on content

    refreshDeviceList();
}

MidiSettingsPanel::~MidiSettingsPanel() = default;

void MidiSettingsPanel::paint (Graphics& g)
{
    g.fillAll (Colour (0xFF2B2D30)); // Slightly lighter than main background
}

void MidiSettingsPanel::resized()
{
    auto r = getLocalBounds().reduced (20);

    titleLabel.setBounds (r.removeFromTop (24));
    r.removeFromTop (4);
    infoLabel.setBounds (r.removeFromTop (20));
    r.removeFromTop (16);

    deviceViewport.setBounds (r);

    // Layout device toggles in container
    auto containerBounds = Rectangle<int> (0, 0, deviceViewport.getWidth() - 20, 0);
    int yPos = 10;

    for (auto* toggle : deviceToggles)
    {
        toggle->setBounds (10, yPos, containerBounds.getWidth() - 20, 28);
        yPos += 36;
    }

    deviceContainer.setSize (containerBounds.getWidth(), yPos + 10);
}

void MidiSettingsPanel::refreshDeviceList()
{
    deviceToggles.clear();

    auto devices = appEngine.listMidiInputDevices();

    if (devices.isEmpty())
    {
        infoLabel.setText ("No MIDI input devices detected", dontSendNotification);
        return;
    }

    infoLabel.setText ("Enable or disable MIDI input devices", dontSendNotification);

    for (const auto& deviceName : devices)
    {
        auto* toggle = new ToggleButton (deviceName);
        toggle->setToggleState (appEngine.isMidiDeviceEnabled (deviceName), dontSendNotification);

        toggle->onClick = [this, deviceName, toggle]
        {
            bool enabled = toggle->getToggleState();
            onDeviceToggled (deviceName, enabled);
        };

        deviceContainer.addAndMakeVisible (toggle);
        deviceToggles.add (toggle);
    }

    resized();
}

void MidiSettingsPanel::onDeviceToggled (const String& deviceName, bool enabled)
{
    appEngine.setMidiDeviceEnabled (deviceName, enabled);
}
