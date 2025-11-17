#pragma once

#include "../../AppEngine/AppEngine.h"
#include "ChannelComponents/ChannelStrip.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace te = tracktion::engine;

class MixerPanel final : public juce::Component
{
public:
    explicit MixerPanel (AppEngine& engine);
    ~MixerPanel() override;

    void refreshTracks();
    void resized() override;
    void refreshArmStates();

private:
    AppEngine& appEngine;

    juce::OwnedArray<ChannelStrip> trackStrips;
    std::unique_ptr<ChannelStrip> masterStrip;

    juce::Viewport tracksViewport;
    juce::Component tracksContainer;

    int innerMargin = 12;
    int gap = 12;
    int stripW = 120;
};
