//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackView/TrackEditView.h"
#include "../AppEngine/AppEngine.h"
#include "TrackEditViewTutorial/TrackEditViewTutorial.h"
#include "../DatabaseManager/DatabaseManager.h"
#include "InstrumentTutorial/InstrumentTutorial.h"
#include "TutorialManager/TutorialManagerComponent.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void showTrackView();
    void showWelcomeView();
    void showTrackViewTutorial();
    void showInstrumentTutorial();


    void reportDatabaseSize();

private:
    std::unique_ptr<Component> view;

    DatabaseManager databaseManager;
};
