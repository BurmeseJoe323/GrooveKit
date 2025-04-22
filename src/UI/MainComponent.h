#pragma once

#include "../AppEngine/AppEngine.h"
#include "../DatabaseManager/DatabaseManager.h"
#include "TrackEditViewTutorial/TrackEditViewTutorial.h"
#include "TrackView/TrackView.h"
#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent final : public Component
{
public:
    explicit MainComponent (AppEngine& engine);
    ~MainComponent() override;

    void paint (Graphics&) override;
    void resized() override;

private:
    AppEngine& appEngine;
    DatabaseManager databaseManager;
    std::unique_ptr<TrackView> trackView;
    std::unique_ptr<TutorialManagerComponent> tutorialManager;

    TextButton openTrackView { "TrackView" };
    TextButton openTutorialManager { "Tutorials" };

    void showTrackView();
    void showTutorialManager();
    void showHome();
};
