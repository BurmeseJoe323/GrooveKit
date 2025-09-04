#ifndef TRACKHEADER_H
#define TRACKHEADER_H

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;
class TrackHeaderComponent : public Component
{
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onAddClipClicked() = 0;
        virtual void onDeleteTrackClicked() = 0;
        virtual void onPianoRollClicked() = 0;
        virtual void onMuteToggled(bool isMuted) = 0;
        virtual void onSoloToggled(bool isSolo) = 0;
        virtual void onDrumSamplerClicked() = 0;
    };

    void addListener(Listener* listener) {listeners.add(listener);}
    void removeListener(Listener* listener) {listeners.remove(listener);}

    TrackHeaderComponent();
    ~TrackHeaderComponent();


    //selection control
    // void mouseDown(const MouseEvent& event) override;
    void setSelected(bool shouldBeSelected);
    // bool isSelected() const;

    void paint(Graphics& g) override;
    void resized() override;
    // void mouseDown(const MouseEvent& event) override;
    // void setSelected(bool shouldBeSelected);
    // bool isSelected() const;

    void setMuted(bool shouldBeMuted);
    bool isMuted() const;

    void setSolo(bool shouldBeSolo);
    bool isSolo() const;
    void setDimmed(bool dim);

    void setTrackName(juce::String name);

private:
    juce::TextButton addClip {"+"};
    juce::TextButton muteTrackButton {"M"};
    juce::TextButton soloTrackButton{ "S" };
    juce::TextButton deleteTrackButton { "-" };
    juce::TextButton pianoRollButton{"Edit clip"};
    juce::TextButton drumSamplerButton{"Drums"};
    juce::Label trackNameLabel {"Track"};
    bool selected = false;

    void updateMuteButtonVisuals();
    void updateSoloButtonVisuals();

    juce::ListenerList<Listener> listeners;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackHeaderComponent)
};

#endif //TRACKHEADER_H
