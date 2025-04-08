//
// Created by ikera on 4/8/2025.
//

#ifndef TRACKHEADER_H
#define TRACKHEADER_H



#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TrackHeader : public juce::Component{
public:
    TrackHeader();
    ~TrackHeader();

    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    juce::TextButton addClip {"Add"};
    juce::Label trackNameLabel {"Track"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackHeader)
};


#endif //TRACKHEADER_H
