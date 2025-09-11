#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackView/TrackEditView.h"
#include "../AppEngine/AppEngine.h"
#include "TrackEditViewTutorial/TrackEditViewTutorial.h"
#include "../DatabaseManager/DatabaseManager.h"
#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent final : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;


    void showWelcomeView();
    void showTrackViewTutorial();
    void showInstrumentTutorial();

    void resized() override;

    void reportDatabaseSize();

private:
    std::unique_ptr<Component> view;
    AppEngine appEngine;

    DatabaseManager databaseManager;
};
