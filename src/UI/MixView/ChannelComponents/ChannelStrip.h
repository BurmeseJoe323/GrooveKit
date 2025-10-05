/// JUNIE
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "ChannelStripComponents/FaderComponent.h"
#include "../TrackView/TrackHeaderComponent.h"

namespace te = tracktion;

class ChannelStrip final : public juce::Component
{
public:
    ChannelStrip();
    ~ChannelStrip() override;

    void bindToTrack (te::AudioTrack& track);
    void bindToMaster (te::Edit& edit);
    void bindToVolume (te::VolumeAndPanPlugin& vnp);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setTrackName (juce::String s) { name.setText (s, juce::dontSendNotification); }

    // Listener passthrough (store as SafePointers to avoid dangling UI references) (Junie)
    void addListener (TrackHeaderComponent::Listener* l)
    {
        if (auto* c = dynamic_cast<juce::Component*>(l))
            listenerComponents.add (juce::Component::SafePointer<juce::Component> (c));
    }
    void removeListener (TrackHeaderComponent::Listener* l)
    {
        if (l == nullptr) return;
        for (int i = listenerComponents.size(); --i >= 0;)
        {
            if (listenerComponents[i] == dynamic_cast<juce::Component*>(l))
                listenerComponents.remove (i);
        }
    }

    // State helpers
    void setMuted (bool isMuted);
    bool isMuted() const;

    void setSolo (bool isSolo);
    bool isSolo() const;

    // void setMeterLevel(float lin)      { meter.setLevel(lin); }
    // void setMeterPeak(float lin)       { meter.setPeak(lin); }

public:
    // Fallback callbacks used when TrackComponents (listeners) are not present (e.g., in Mix view) (Junie)
    std::function<void (bool)> onRequestMuteChange;
    std::function<void (bool)> onRequestSoloChange;

private:
    juce::TextButton muteButton, soloButton, recordButton;
    juce::Label name;
    //ChannelMeter     meter;
    FaderComponent lnf;
    juce::Slider fader;
    juce::Slider pan;

    te::VolumeAndPanPlugin* boundVnp { nullptr };
    bool ignoreSliderCallback { false };

    juce::Array<juce::Component::SafePointer<juce::Component>> listenerComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStrip)
};
