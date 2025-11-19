// Created by Claude Code on 2025-11-18.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

/**
 * MIDI settings panel - enable/disable MIDI input devices
 */
class MidiSettingsPanel : public juce::Component
{
public:
    explicit MidiSettingsPanel (AppEngine& engine);
    ~MidiSettingsPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void refreshDeviceList();
    void onDeviceToggled (const juce::String& deviceName, bool enabled);

    AppEngine& appEngine;

    juce::Label titleLabel { {}, "MIDI Input Devices:" };
    juce::Label infoLabel { {}, "Enable or disable MIDI input devices" };

    juce::OwnedArray<juce::ToggleButton> deviceToggles;
    juce::Viewport deviceViewport;
    juce::Component deviceContainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiSettingsPanel)
};
