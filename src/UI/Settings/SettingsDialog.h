// Created by Claude Code on 2025-11-18.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;
class AudioSettingsPanel;
class MidiSettingsPanel;

/**
 * Main settings dialog with tabbed interface for Audio and MIDI settings
 */
class SettingsDialog : public juce::Component
{
public:
    explicit SettingsDialog (AppEngine& engine);
    ~SettingsDialog() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    AppEngine& appEngine;

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };

    std::unique_ptr<AudioSettingsPanel> audioPanel;
    std::unique_ptr<MidiSettingsPanel> midiPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsDialog)
};
