//
// Created by ikera on 4/8/2025.
//

#ifndef TRACKCLIP_H
#define TRACKCLIP_H




#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TrackClip : public juce::Component{
public:
    TrackClip();
    ~TrackClip();

    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    // below are examples of creating buttons and labels
    // juce::TextButton addClip {"simpl"};
    // juce::Label trackNameLabel {"Track"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackClip)
};


#endif //TRACKCLIP_H
