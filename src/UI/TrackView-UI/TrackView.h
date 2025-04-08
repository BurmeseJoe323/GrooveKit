//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "EditComponent.h"
#include "../../AppEngine/AppEngine.h"
#include "TrackComponent.h"

using namespace juce;
/// TrackView will provide all of the necessary UI features of a
/// track view, Tracks, edit and loading, and many other features
/// This will be done by including many subcomponents within the
/// TrackView Component
/// Example:
///     - TrackComponent
class TrackView : public juce::Component {
public:
    explicit TrackView(AppEngine& engine);
    ~TrackView() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    TextButton newEditButton { "New" }, playPauseButton {"Play"}, stopButton {"Stop"}, deleteButton { "Delete" },
               openEditButton {"Open Edit"}, newTrackButton { "New Track" }, recordButton { "Record" };

    std::unique_ptr<EditComponent> editComponent;
    AppEngine& appEngine;
};