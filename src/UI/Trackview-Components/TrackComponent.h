//
// Created by ikera on 4/6/2025.
//

#ifndef TRACKCOMPONENT_H
#define TRACKCOMPONENT_H


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TrackComponent : public juce::Component{
public:
    TrackComponent();
    ~TrackComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};



#endif //TRACKCOMPONENT_H
