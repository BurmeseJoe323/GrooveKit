#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../AppEngine/AppEngine.h"
#include "ChannelComponents/ChannelStrip.h"

namespace te = tracktion;

class MixerPanel : public juce::Component
{
public:
    MixerPanel (AppEngine& engine);
    ~MixerPanel();

    void refreshTracks();
    void resized() override;
    void refreshArmStates();

private:
    AppEngine& appEngine;

    juce::OwnedArray<ChannelStrip> trackStrips;
    std::unique_ptr<ChannelStrip> masterStrip;

    int innerMargin = 12;
    int gap = 12;
    int stripW = 120;
};
