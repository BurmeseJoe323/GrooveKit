#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

struct DrumSamplerEngine
{
    virtual ~DrumSamplerEngine() = default;

    // slots: 0..15 for 4x4
    virtual void loadSampleIntoSlot (int slot, const juce::File& file) = 0;
    virtual void triggerSlot        (int slot, float velocity) = 0;

    // Global params
    virtual void setVolume (float linear01) = 0; // 0..1
    virtual void setADSR   (float a, float d, float s, float r) = 0;

    // Optional: query names
    virtual juce::String getSlotName (int slot) const = 0;
};
