// Created by Claude Code on 2025-11-18.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

/**
 * Audio settings panel - output device, sample rate, and buffer size configuration
 */
class AudioSettingsPanel : public juce::Component, private juce::ComboBox::Listener
{
public:
    explicit AudioSettingsPanel (AppEngine& engine);
    ~AudioSettingsPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void comboBoxChanged (juce::ComboBox* combo) override;
    void refreshDeviceList();
    void refreshBufferSizes();
    void refreshSampleRates();

    AppEngine& appEngine;

    // Output device controls
    juce::Label deviceLabel { {}, "Output Device:" };
    juce::ComboBox deviceCombo;
    juce::TextButton refreshDeviceBtn { "Refresh" };
    juce::TextButton defaultDeviceBtn { "Use System Default" };

    // Sample rate controls
    juce::Label sampleRateLabel { {}, "Sample Rate:" };
    juce::ComboBox sampleRateCombo;

    // Buffer size controls
    juce::Label bufferSizeLabel { {}, "Buffer Size:" };
    juce::ComboBox bufferSizeCombo;
    juce::Label latencyLabel { {}, "" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettingsPanel)
};
