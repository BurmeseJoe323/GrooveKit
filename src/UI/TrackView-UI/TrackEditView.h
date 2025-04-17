//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackListComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

/// TrackView will provide all of the necessary UI features of a
/// track view, Tracks, edit and loading, and many other features
/// This will be done by including many subcomponents within the
/// TrackView Component
/// Example:
///     - TrackComponent
class TrackEditView final : public Component
{
public:
    void setupButtons();
    explicit TrackEditView (AppEngine& engine);
    ~TrackEditView() override;

    void paint (Graphics&) override;
    void resized() override;

private:
    AppEngine& appEngine;
    std::unique_ptr<TrackListComponent> trackList;
    Viewport viewport;

    TextButton newEditButton { "New" }, playPauseButton { "Play" }, stopButton { "Stop" }, deleteButton { "Delete" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, recordButton { "Record" };
};