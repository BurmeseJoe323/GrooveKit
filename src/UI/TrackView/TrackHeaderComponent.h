#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

class TrackHeaderComponent final : public juce::Component,
                                   public juce::Label::Listener
{
public:
    enum class TrackType { Drum,
        Instrument };

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void onSettingsClicked() = 0;
        virtual void onMuteToggled (bool isMuted) = 0;
        virtual void onSoloToggled (bool isSolo) = 0;
        virtual void onInstrumentClicked() = 0;
        virtual void onInstrumentMenuRequested() = 0;
        virtual void onRecordArmToggled(bool isArmed) = 0;
    };

    void addListener (Listener* listener) { listeners.add (listener); }
    void removeListener (Listener* listener) { listeners.remove (listener); }

    explicit TrackHeaderComponent (AppEngine& engine);
    ~TrackHeaderComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setMuted (bool shouldBeMuted);
    bool isMuted() const;

    void setSolo (bool shouldBeSolo);
    bool isSolo() const;

    void setArmed (bool shouldBeArmed);
    bool isArmed() const;
    void setArmButtonEnabled(bool enabled);
    void setDimmed (bool dim);

    void setTrackName (juce::String name);
    void setTrackType (TrackType type);

    void setTrackIndex (int newIndex);
    void setInstrumentLabel (const juce::String& text);

    void refreshInstrumentButton();

    // Track index management (Written by Claude Code)
    int getTrackIndex() const { return trackIndex; }

    // juce::Label::Listener implementation (Written by Claude Code)
    void labelTextChanged (juce::Label* labelThatHasChanged) override;

private:
    AppEngine* appEngine = nullptr;

    int trackIndex { -1 };

    juce::TextButton instrumentButton;
    juce::TextButton instrumentMenuButton;
    juce::TextButton settingsButton { "..." };
    juce::TextButton muteTrackButton { "M" };
    juce::TextButton soloTrackButton { "S" };
    juce::TextButton recordArmButton { "R" };
    juce::Label trackNameLabel { "Track" };
    bool selected = false;

    TrackType trackType = TrackType::Instrument;
    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackHeaderComponent)
};
