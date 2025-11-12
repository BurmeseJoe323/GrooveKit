#pragma once

#include "../TrackView/TrackHeaderComponent.h"
#include "ChannelStripComponents/FaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

class ChannelStrip final : public juce::Component,
                           public juce::Label::Listener
{
public:
    explicit ChannelStrip(juce::Colour color = juce::Colour(0xFF495057));
    ~ChannelStrip() override;

    void bindToTrack (te::AudioTrack& track);
    void bindToMaster (te::Edit& edit);
    void bindToVolume (te::VolumeAndPanPlugin& vnp);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setTrackName (const juce::String& s) { name.setText (s, juce::dontSendNotification); }

    // Track index management (Written by Claude Code)
    void setTrackIndex (int index) { trackIndex = index; }
    int getTrackIndex() const { return trackIndex; }

    // juce::Label::Listener implementation (Written by Claude Code)
    void labelTextChanged (juce::Label* labelThatHasChanged) override;

    // Listener passthrough (store as SafePointers to avoid dangling UI references) (Junie)
    void addListener (TrackHeaderComponent::Listener* l)
    {
        if (auto* c = dynamic_cast<juce::Component*> (l))
            listenerComponents.add (juce::Component::SafePointer<juce::Component> (c));
    }
    void removeListener (TrackHeaderComponent::Listener* l)
    {
        if (l == nullptr)
            return;
        for (int i = listenerComponents.size(); --i >= 0;)
        {
            if (listenerComponents[i] == dynamic_cast<juce::Component*> (l))
                listenerComponents.remove (i);
        }
    }

    // State helpers
    void setMuted (bool isMuted);
    bool isMuted() const;

    void setSolo (bool isSolo);
    bool isSolo() const;

    void setArmed (bool isArmed);
    bool isArmed() const;

    // void setMeterLevel(float lin)      { meter.setLevel(lin); }
    // void setMeterPeak(float lin)       { meter.setPeak(lin); }

public:
    // Fallback callbacks used when TrackComponents (listeners) are not present (e.g., in Mix view) (Junie)
    std::function<void (bool)> onRequestMuteChange;
    std::function<void (bool)> onRequestSoloChange;
    std::function<void (bool)> onRequestArmChange;
    std::function<void (te::AudioTrack&)> onInstrumentClicked;
    std::function<void()> onOpenInstrumentEditor;
    // Track naming callback (Written by Claude Code)
    std::function<void (int trackIndex, const juce::String& newName)> onRequestNameChange;

private:
    int trackIndex = -1; // Track index for this channel strip (Written by Claude Code)
    juce::Colour stripColor; // Background color for this strip (Written by Claude Code)
    juce::TextButton muteButton, soloButton, recordButton;
    juce::TextButton instrumentButton;
    juce::Label insertsLabel;
    juce::OwnedArray<juce::TextButton> insertSlots;
    juce::Label name;
    //ChannelMeter     meter;
    FaderComponent lnf;
    juce::Slider fader;
    juce::Slider pan;
    te::AudioTrack* boundTrack { nullptr };

    te::VolumeAndPanPlugin* boundVnp { nullptr };
    bool ignoreSliderCallback { false };

    juce::Array<juce::Component::SafePointer<juce::Component>> listenerComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStrip)
};
