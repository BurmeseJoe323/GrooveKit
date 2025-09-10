#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

#include "ChannelStripComponents/FaderComponent.h"
namespace te = tracktion;
class ChannelStrip : public juce::Component
{
public:
    ChannelStrip();
    ~ChannelStrip() override;

    void bindToTrack(te::AudioTrack& track);
    void bindToMaster(te::Edit& edit);
    void bindToVolume(te::VolumeAndPanPlugin& vnp);


    void paint(juce::Graphics& g) override;

    void resized() override;

    void setTrackName(juce::String s) { name.setText(s, juce::dontSendNotification); }
    // void setMeterLevel(float lin)      { meter.setLevel(lin); }
    // void setMeterPeak(float lin)       { meter.setPeak(lin); }

private:
    juce::TextButton muteButton, soloButton, recordButton;
    juce::Label      name;
    //ChannelMeter     meter;
    FaderComponent   lnf;

    juce::Slider     fader;
    juce::Slider     pan;
    te::VolumeAndPanPlugin::Ptr boundVnp;
    bool ignoreSliderCallback { false };
};
