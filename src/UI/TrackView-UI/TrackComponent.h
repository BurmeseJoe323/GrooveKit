//
// Created by ikera on 4/6/2025.
//

#ifndef TRACKCOMPONENT_H
#define TRACKCOMPONENT_H


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackHeader.h"
#include "TrackClip.h"
#include "../../AppEngine/AppEngine.h"

///TrackComponent provides track UI element that contains the following
///   - TrackHeader Component
///   - TrackClips Component
class TrackComponent : public juce::Component, public TrackHeader::Listener {
public:
    TrackComponent(std::shared_ptr<AppEngine> engine, int trackIndex);
    ~TrackComponent() override;

    void onAddClipClicked() override;
    void onDeleteTrackClicked() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void(int)> onRequestDeleteTrack;

private:
    std::shared_ptr<AppEngine> appEngine;
    int trackIndex;

    TrackClip trackClip;
    TrackHeader trackHeader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackComponent)
};



#endif //TRACKCOMPONENT_H
