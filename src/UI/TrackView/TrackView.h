#pragma once

#include "../../AppEngine/AppEngine.h"
#include "EditComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

class TrackView final : public Component
{
public:
    explicit TrackView (AppEngine& engine);
    ~TrackView() override;

    void paint (Graphics&) override;
    void resized() override;

    /**
     Called when Back is pressed: should return to home screen.
    */
    std::function<void()> onBack;

private:
    TextButton backButton { "Back" }, newEditButton { "New Edit" }, playPauseButton { "Play" }, stopButton { "Stop" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, recordButton { "Record" };

    std::unique_ptr<EditComponent> editComponent;
    AppEngine& appEngine;
};