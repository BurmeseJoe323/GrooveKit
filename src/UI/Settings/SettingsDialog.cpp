// Created by Claude Code on 2025-11-18.
#include "SettingsDialog.h"
#include "AudioSettingsPanel.h"
#include "MidiSettingsPanel.h"
#include "../../AppEngine/AppEngine.h"

SettingsDialog::SettingsDialog (AppEngine& engine)
    : appEngine (engine)
{
    addAndMakeVisible (tabs);

    // Create and add panels as tabs
    audioPanel = std::make_unique<AudioSettingsPanel> (appEngine);
    midiPanel = std::make_unique<MidiSettingsPanel> (appEngine);

    tabs.addTab ("Audio", juce::Colours::darkgrey, audioPanel.get(), false);
    tabs.addTab ("MIDI", juce::Colours::darkgrey, midiPanel.get(), false);

    tabs.setCurrentTabIndex (0);
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF343A40)); // Match GrooveKit dark background
}

void SettingsDialog::resized()
{
    tabs.setBounds (getLocalBounds());
}
