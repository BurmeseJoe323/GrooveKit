//
// Created by ikera on 4/9/2025.
//

#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackComponent.h"
#include "TrackHeaderComponent.h"

/**
 * Represents the Component view holding tracks.
 */
class TrackListComponent final : public Component
{
public:
    explicit TrackListComponent (std::shared_ptr<AppEngine> engine);
    ~TrackListComponent() override;

    void paint (Graphics& g) override;
    void resized() override;
    void parentSizeChanged() override;
    void addNewTrack (int index);

private:
    std::shared_ptr<AppEngine> appEngine;

    OwnedArray<TrackComponent> tracks;
    OwnedArray<TrackHeaderComponent> headers;
    // OwnedArray<TrackFooterComponent> footers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackListComponent)
};