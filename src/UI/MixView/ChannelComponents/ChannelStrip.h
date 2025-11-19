#pragma once

#include "../TrackView/TrackHeaderComponent.h"
#include "ChannelStripComponents/FaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * @brief A single mixer-strip UI component used in Mix View.
 *
 * ChannelStrip represents either:
 *  - A regular AudioTrack (bindToTrack), or
 *  - The Master output channel (bindToMaster), or
 *  - A standalone VolumeAndPanPlugin (bindToVolume).
 *
 * It contains:
 *   - Instrument button
 *   - FX Insert slots with ▼ menu buttons
 *   - Mute / Solo / Record-Arm buttons
 *   - Volume fader (custom LookAndFeel)
 *   - Pan knob
 *   - Editable track name label
 *
 * It forwards user actions back to TrackHeaderComponent listeners or to
 * callback functions provided by MixView (e.g., onRequestMuteChange).
 */
class ChannelStrip final : public juce::Component,
                           public juce::Label::Listener
{
public:
    explicit ChannelStrip (juce::Colour color = juce::Colour (0xFF495057));
    ~ChannelStrip() override;

    //==========================================================================
    /** Bind this strip to a specific AudioTrack so UI reflects its state. */
    void bindToTrack (te::AudioTrack& track);

    /** Bind strip to the master edit output (no instrument button, shared VNP). */
    void bindToMaster (te::Edit& edit);

    /** Bind strip to an arbitrary VolumeAndPanPlugin. */
    void bindToVolume (te::VolumeAndPanPlugin& vnp);

    //==========================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    /** Sets displayed name text WITHOUT sending rename callbacks. */
    void setTrackName (const juce::String& s) { name.setText (s, juce::dontSendNotification); }

    // Track index helpers (stored so MixView knows which strip this is)
    void setTrackIndex (int index) { trackIndex = index; }
    int  getTrackIndex() const     { return trackIndex; }

    //==========================================================================
    /** Handle user-entered track name changes. */
    void labelTextChanged (juce::Label* labelThatHasChanged) override;

    //==========================================================================
    /**
     * @brief Register a TrackHeaderComponent listener (via SafePointer).
     *
     * MixView uses this to keep header + strip in sync.
     */
    void addListener (TrackHeaderComponent::Listener* l)
    {
        if (auto* c = dynamic_cast<juce::Component*> (l))
            listenerComponents.add (juce::Component::SafePointer<juce::Component> (c));
    }

    /** Remove a previously added listener. */
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

    //==========================================================================
    // State helpers (sync UI without re-triggering callbacks)
    void setMuted (bool isMuted);
    bool isMuted() const;

    void setSolo (bool isSolo);
    bool isSolo() const;

    void setArmed (bool isArmed);
    bool isArmed() const;

    /** Change label of a specific FX insert slot (0–3). */
    void setInsertSlotName (int slotIndex, const juce::String& text);

    /** Sets the Instrument button text (fallback = "Instrument"). */
    void setInstrumentButtonText (const juce::String& text);

    int getNumInsertSlots() const { return insertSlots.size(); }

    //==========================================================================
    // Callback hooks used by MixView when TrackHeaderComponent is not present
    std::function<void (bool)> onRequestMuteChange;
    std::function<void (bool)> onRequestSoloChange;
    std::function<void (bool)> onRequestArmChange;
    std::function<void()>      onOpenInstrumentEditor;
    std::function<void (int)>  onInsertSlotClicked;
    std::function<void (int)>  onInsertSlotMenuRequested;
    std::function<void (int trackIndex, const juce::String& newName)> onRequestNameChange;

private:
    // UI + state
    int trackIndex = -1;
    juce::Colour stripColor;

    juce::TextButton muteButton, soloButton, recordButton;
    juce::TextButton instrumentButton;

    juce::Label insertsLabel;
    juce::OwnedArray<juce::TextButton> insertSlots;
    juce::OwnedArray<juce::TextButton> insertSlotMenus;

    juce::Label name;

    FaderComponent lnf;
    juce::Slider fader;
    juce::Slider pan;

    te::AudioTrack*           boundTrack { nullptr };
    te::VolumeAndPanPlugin*   boundVnp   { nullptr };
    bool ignoreSliderCallback { false };

    juce::Array<juce::Component::SafePointer<juce::Component>> listenerComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStrip)
};