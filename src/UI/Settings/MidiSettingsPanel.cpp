// Created by Claude Code on 2025-11-18.
#include "MidiSettingsPanel.h"
#include "../../AppEngine/AppEngine.h"

using namespace juce;

MidiSettingsPanel::MidiSettingsPanel (AppEngine& engine)
    : appEngine (engine)
{
    addAndMakeVisible (titleLabel);
    titleLabel.setJustificationType (Justification::centredLeft);
    titleLabel.setFont (Font (FontOptions (14.0f, Font::bold)));

    addAndMakeVisible (infoLabel);
    infoLabel.setJustificationType (Justification::centredLeft);
    infoLabel.setFont (Font (FontOptions (12.0f)));
    infoLabel.setColour (Label::textColourId, Colours::grey);

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

    // Layout device labels in container
    auto containerBounds = Rectangle<int> (0, 0, deviceViewport.getWidth() - 20, 0);
    int yPos = 10;

    for (auto* label : deviceLabels)
    {
        label->setBounds (10, yPos, containerBounds.getWidth() - 20, 24);
        yPos += 30;
    }

    deviceContainer.setSize (containerBounds.getWidth(), yPos + 10);
}

void MidiSettingsPanel::refreshDeviceList()
{
    deviceLabels.clear();

    auto devices = appEngine.listMidiInputDevices();

    if (devices.isEmpty())
    {
        infoLabel.setText ("No MIDI input devices detected", dontSendNotification);

        // Add a placeholder label
        auto* noDeviceLabel = new Label();
        noDeviceLabel->setText ("No devices found", dontSendNotification);
        noDeviceLabel->setJustificationType (Justification::centredLeft);
        noDeviceLabel->setFont (Font (FontOptions (13.0f)));
        noDeviceLabel->setColour (Label::textColourId, Colours::grey);

        deviceContainer.addAndMakeVisible (noDeviceLabel);
        deviceLabels.add (noDeviceLabel);

        resized();
        return;
    }

    infoLabel.setText (String (devices.size()) + " device" + (devices.size() == 1 ? "" : "s") + " detected", dontSendNotification);

    for (const auto& deviceName : devices)
    {
        auto* label = new Label();
        label->setText ("• " + deviceName, dontSendNotification);
        label->setJustificationType (Justification::centredLeft);
        label->setFont (Font (FontOptions (13.0f)));
        label->setColour (Label::textColourId, Colours::white);

        deviceContainer.addAndMakeVisible (label);
        deviceLabels.add (label);
    }

    resized();
}
