//
// Created by ikera on 4/6/2025.
//

#ifndef TRACKCOMPONENT_H
#define TRACKCOMPONENT_H


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackHeader.h"
#include "TrackClip.h"

///TrackComponent provides track UI element that contains the following
///   - TrackHeader Component
///   - TrackClips Component
class TrackComponent : public juce::Component, public TrackHeader::Listener{
public:
    TrackComponent();
    ~TrackComponent();

    /// adds a clip component to the UI.
    void onAddClipClicked() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    TrackClip trackClip;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};



#endif //TRACKCOMPONENT_H
