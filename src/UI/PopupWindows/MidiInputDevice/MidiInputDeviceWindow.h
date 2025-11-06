#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

class MidiInputDeviceWindow : public juce::Component, private juce::ComboBox::Listener
{
public:
    explicit MidiInputDeviceWindow (AppEngine& appEng);
    ~MidiInputDeviceWindow() override = default;

    void resized() override;

private:
    void comboBoxChanged (juce::ComboBox* c) override;
    void refreshList();

    AppEngine& app;
    juce::Label   title   { {}, "MIDI input device" };
    juce::ComboBox devices;
    juce::TextButton refreshBtn { "Refresh" };
    juce::TextButton disableBtn { "Disable MIDI Input" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInputDeviceWindow)
};
