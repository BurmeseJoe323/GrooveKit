// Created by Claude Code on 2025-11-18.
#include "AudioSettingsPanel.h"
#include "../../AppEngine/AppEngine.h"

using namespace juce;

AudioSettingsPanel::AudioSettingsPanel (AppEngine& engine)
    : appEngine (engine)
{
    // Output device controls
    addAndMakeVisible (deviceLabel);
    deviceLabel.setJustificationType (Justification::centredLeft);
    deviceLabel.setFont (Font (FontOptions (14.0f, Font::bold)));

    addAndMakeVisible (deviceCombo);
    deviceCombo.addListener (this);

    addAndMakeVisible (refreshDeviceBtn);
    refreshDeviceBtn.onClick = [this] { refreshDeviceList(); };

    addAndMakeVisible (defaultDeviceBtn);
    defaultDeviceBtn.onClick = [this]
    {
        appEngine.setDefaultOutputDevice();
        refreshDeviceList();
        refreshSampleRates();
        refreshBufferSizes();
    };

    // Sample rate controls
    addAndMakeVisible (sampleRateLabel);
    sampleRateLabel.setJustificationType (Justification::centredLeft);
    sampleRateLabel.setFont (Font (FontOptions (14.0f, Font::bold)));

    addAndMakeVisible (sampleRateCombo);
    sampleRateCombo.addListener (this);

    // Buffer size controls
    addAndMakeVisible (bufferSizeLabel);
    bufferSizeLabel.setJustificationType (Justification::centredLeft);
    bufferSizeLabel.setFont (Font (FontOptions (14.0f, Font::bold)));

    addAndMakeVisible (bufferSizeCombo);
    bufferSizeCombo.addListener (this);

    addAndMakeVisible (latencyLabel);
    latencyLabel.setJustificationType (Justification::centredLeft);
    latencyLabel.setFont (Font (FontOptions (13.0f)));
    latencyLabel.setColour (Label::textColourId, Colours::lightgrey);

    // Initialize with current settings
    refreshDeviceList();
    refreshSampleRates();
    refreshBufferSizes();
}

AudioSettingsPanel::~AudioSettingsPanel()
{
    deviceCombo.removeListener (this);
    sampleRateCombo.removeListener (this);
    bufferSizeCombo.removeListener (this);
}

void AudioSettingsPanel::paint (Graphics& g)
{
    g.fillAll (Colour (0xFF2B2D30)); // Slightly lighter than main background
}

void AudioSettingsPanel::resized()
{
    auto r = getLocalBounds().reduced (20);

    // Output device section
    deviceLabel.setBounds (r.removeFromTop (24));
    r.removeFromTop (8);
    deviceCombo.setBounds (r.removeFromTop (28));
    r.removeFromTop (8);

    auto deviceButtonRow = r.removeFromTop (28);
    refreshDeviceBtn.setBounds (deviceButtonRow.removeFromLeft (120));
    deviceButtonRow.removeFromLeft (10);
    defaultDeviceBtn.setBounds (deviceButtonRow.removeFromLeft (160));

    r.removeFromTop (20);

    // Sample rate section
    sampleRateLabel.setBounds (r.removeFromTop (24));
    r.removeFromTop (8);
    sampleRateCombo.setBounds (r.removeFromTop (28));

    r.removeFromTop (20);

    // Buffer size section
    bufferSizeLabel.setBounds (r.removeFromTop (24));
    r.removeFromTop (8);
    bufferSizeCombo.setBounds (r.removeFromTop (28));
    r.removeFromTop (8);
    latencyLabel.setBounds (r.removeFromTop (20));
}

void AudioSettingsPanel::comboBoxChanged (ComboBox* combo)
{
    if (combo == &deviceCombo)
    {
        auto chosen = deviceCombo.getText();
        if (chosen.isNotEmpty())
        {
            appEngine.setOutputDevice (chosen);
            refreshSampleRates();
            refreshBufferSizes();
        }
    }
    else if (combo == &sampleRateCombo)
    {
        auto selectedRate = sampleRateCombo.getText().getDoubleValue();
        if (selectedRate > 0)
        {
            appEngine.setSampleRate (selectedRate);
            refreshBufferSizes(); // Update latency calculation
        }
    }
    else if (combo == &bufferSizeCombo)
    {
        auto selectedBuffer = bufferSizeCombo.getText().getIntValue();
        if (selectedBuffer > 0)
        {
            appEngine.setBufferSize (selectedBuffer);

            // Update latency display
            auto sampleRate = appEngine.getCurrentSampleRate();
            if (sampleRate > 0)
            {
                double latencyMs = (selectedBuffer / sampleRate) * 1000.0;
                latencyLabel.setText ("Latency: " + String (latencyMs, 1) + " ms",
                                      dontSendNotification);
            }
        }
    }
}

void AudioSettingsPanel::refreshDeviceList()
{
    deviceCombo.clear();

    auto devices = appEngine.listOutputDevices();
    for (int i = 0; i < devices.size(); ++i)
        deviceCombo.addItem (devices[i], i + 1);

    auto current = appEngine.getCurrentOutputDeviceName();
    if (current.isNotEmpty())
        deviceCombo.setText (current, dontSendNotification);
    else if (deviceCombo.getNumItems() > 0)
        deviceCombo.setSelectedItemIndex (0, dontSendNotification);
}

void AudioSettingsPanel::refreshSampleRates()
{
    sampleRateCombo.clear();

    auto rates = appEngine.getAvailableSampleRates();
    for (int i = 0; i < rates.size(); ++i)
    {
        auto rate = rates[i];
        String rateStr = String (rate / 1000.0, 1) + " kHz";
        sampleRateCombo.addItem (rateStr, i + 1);
    }

    auto currentRate = appEngine.getCurrentSampleRate();
    if (currentRate > 0)
    {
        String currentStr = String (currentRate / 1000.0, 1) + " kHz";
        sampleRateCombo.setText (currentStr, dontSendNotification);
    }
}

void AudioSettingsPanel::refreshBufferSizes()
{
    bufferSizeCombo.clear();

    auto sizes = appEngine.getAvailableBufferSizes();
    for (int i = 0; i < sizes.size(); ++i)
    {
        auto size = sizes[i];
        bufferSizeCombo.addItem (String (size) + " samples", i + 1);
    }

    auto currentBuffer = appEngine.getCurrentBufferSize();
    if (currentBuffer > 0)
    {
        bufferSizeCombo.setText (String (currentBuffer) + " samples", dontSendNotification);

        // Calculate and display latency
        auto sampleRate = appEngine.getCurrentSampleRate();
        if (sampleRate > 0)
        {
            double latencyMs = (currentBuffer / sampleRate) * 1000.0;
            latencyLabel.setText ("Latency: " + String (latencyMs, 1) + " ms",
                                  dontSendNotification);
        }
    }
}
