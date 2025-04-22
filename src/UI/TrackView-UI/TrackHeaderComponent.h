//
// Created by ikera on 4/8/2025.
//

#ifndef TRACKHEADER_H
#define TRACKHEADER_H



#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
using namespace juce;
class TrackHeaderComponent : public Component{
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onAddClipClicked() = 0;
        virtual void onDeleteTrackClicked() = 0;
        virtual void onPianoRollClicked() = 0;
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

private:
    juce::TextButton addClip {"Add"};
    juce::TextButton deleteTrackButton { "Delete" };
    juce::TextButton pianoRollButton{"Edit clip"};
    juce::Label trackNameLabel {"Track"};
    bool selected = false;

    juce::ListenerList<Listener> listeners;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackHeaderComponent)
};


#endif //TRACKHEADER_H
