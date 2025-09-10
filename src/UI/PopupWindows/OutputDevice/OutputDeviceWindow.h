#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

class OutputDeviceWindow : public juce::Component, private juce::ComboBox::Listener
{
public:
    explicit OutputDeviceWindow (AppEngine& appEng);
    ~OutputDeviceWindow() override = default;

    void resized() override;

private:
    void comboBoxChanged (juce::ComboBox* c) override;
    void refreshList();

    AppEngine& app;
    juce::Label   title   { {}, "Output device" };
    juce::ComboBox devices;
    juce::TextButton refreshBtn { "Refresh" };
    juce::TextButton defaultBtn { "Use System Default" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputDeviceWindow)
};
