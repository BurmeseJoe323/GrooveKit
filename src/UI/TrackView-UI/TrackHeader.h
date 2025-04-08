//
// Created by ikera on 4/8/2025.
//

#ifndef TRACKHEADER_H
#define TRACKHEADER_H



#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TrackHeader : public juce::Component{
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onAddClipClicked() = 0;
    };

    void addListener(Listener* listener) {listeners.add(listener);}
    void removeListener(Listener* listener) {listeners.remove(listener);}

    TrackHeader();
    ~TrackHeader();

    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    juce::TextButton addClip {"Add"};
    juce::Label trackNameLabel {"Track"};

    juce::ListenerList<Listener> listeners;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackHeader)
};


#endif //TRACKHEADER_H
