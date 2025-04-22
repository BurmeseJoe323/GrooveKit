//
// Created by ikera on 4/9/2025.
//

#ifndef EDITCOMPONENT_H
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackComponent.h"
#include "TrackHeader.h"
#include "../../AppEngine/AppEngine.h"

using namespace juce;
class EditComponent : public juce::Component{
public:
    explicit EditComponent(AppEngine& engine);
    ~EditComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void addNewTrack(int index);
private:
    // below are examples of creating buttons and labels
    // juce::TextButton addClip {"simpl"};
    // juce::Label trackNameLabel {"Track"};

    // void removeSelectedTracks();
    // bool keyPressed(const KeyPress& key) override;
    AppEngine& appEngine;
    OwnedArray<TrackComponent> tracks;
    OwnedArray<TrackHeader> headers;
    // OwnedArray<TrackFooterComponent> footers;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditComponent)
};


#endif //EDITCOMPONENT_H