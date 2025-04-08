//
// Created by ikera on 4/6/2025.
//

#ifndef TRACKCOMPONENT_H
#define TRACKCOMPONENT_H


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackHeader.h"

///TrackComponent provides track UI element that contains the following
///   - TrackHeader Component
///   - TrackClips Component
class TrackComponent : public juce::Component{
public:
    TrackComponent();
    ~TrackComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    TrackHeader trackHeader;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};



#endif //TRACKCOMPONENT_H
