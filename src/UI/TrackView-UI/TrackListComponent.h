//
// Created by ikera on 4/9/2025.
//

#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackComponent.h"
#include "TrackHeader.h"
#include <juce_gui_basics/juce_gui_basics.h>

class TrackListComponent final : public Component
{
public:
    explicit TrackListComponent (AppEngine& engine);
    ~TrackListComponent() override;

    void paint (Graphics& g) override;
    void resized() override;
    void parentSizeChanged() override;
    void addNewTrack (int index);

private:
    AppEngine& appEngine;

    OwnedArray<TrackComponent> tracks;
    OwnedArray<TrackHeader> headers;
    // OwnedArray<TrackFooterComponent> footers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackListComponent)
};