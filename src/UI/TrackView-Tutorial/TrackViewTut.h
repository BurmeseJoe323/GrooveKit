//
// Created by ikera on 4/14/2025.
//

#ifndef TRACKVIEWTUT_H
#define TRACKVIEWTUT_H

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TrackViewTut : public juce::Component{
public:
    TrackViewTut();
    ~TrackViewTut() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    juce::TextButton goBack {"Home"};
    // below are examples of creating buttons and labels
    // juce::TextButton addClip {"simpl"};
    // juce::Label trackNameLabel {"Track"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackViewTut)
};


#endif //TRACKVIEWTUT_H
