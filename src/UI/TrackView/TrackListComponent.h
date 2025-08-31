#pragma once

#include "PlayheadComponent.h"
#include "../../AppEngine/AppEngine.h"
#include "TrackComponent.h"
#include "TrackHeaderComponent.h"
#include "../PopupWindows/PianoRollWindow.h"

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
    void refreshSoloVisuals();


    void addNewTrack (int index);

private:
    const std::shared_ptr<AppEngine> appEngine;
    int selectedTrackIndex;

    OwnedArray<TrackComponent> tracks;
    OwnedArray<TrackHeaderComponent> headers;

    PlayheadComponent playhead;

    std::unique_ptr<PianoRollWindow> pianoRollWindow;
    // OwnedArray<TrackFooterComponent> footers;

    void updateTrackIndexes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackListComponent)
};