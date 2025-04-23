//
// Created by ikera on 4/6/2025.
//

#ifndef TRACKCOMPONENT_H
#define TRACKCOMPONENT_H


#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackClip.h"
#include "TrackHeaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

///TrackComponent provides track UI element that contains the following
///   - TrackHeader Component
///   - TrackClips Component
class TrackComponent : public juce::Component, public TrackHeaderComponent::Listener {
public:
    TrackComponent(std::shared_ptr<AppEngine> engine, int trackIndex);
    ~TrackComponent() override;

    void onAddClipClicked() override;
    // int getNumClips();
    void onDeleteTrackClicked() override;
    void onPianoRollClicked() override;

    void setTrackIndex(int index);

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void(int)> onRequestDeleteTrack;
    std::function<void(int)> onRequestOpenPianoRoll;

private:
    std::shared_ptr<AppEngine> appEngine;
    int trackIndex;
    int numClips;

    TrackClip trackClip;
    TrackHeaderComponent trackHeader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackComponent)
};



#endif //TRACKCOMPONENT_H
