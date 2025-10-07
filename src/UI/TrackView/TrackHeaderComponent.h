#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TrackHeaderComponent final : public juce::Component
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
    };

    void addListener (Listener* listener) { listeners.add (listener); }
    void removeListener (Listener* listener) { listeners.remove (listener); }

    TrackHeaderComponent();
    ~TrackHeaderComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setMuted (bool shouldBeMuted);
    bool isMuted() const;

    void setSolo (bool shouldBeSolo);
    bool isSolo() const;
    void setDimmed (bool dim);

    void setTrackName (juce::String name);
    void setTrackType (TrackType type);

private:
    juce::TextButton settingsButton { "..." };
    juce::TextButton muteTrackButton { "M" };
    juce::TextButton soloTrackButton { "S" };
    juce::Label trackNameLabel { "Track" };
    bool selected = false;

    TrackType trackType = TrackType::Instrument;
    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackHeaderComponent)
};
