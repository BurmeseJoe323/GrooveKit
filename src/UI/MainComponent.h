#pragma once

#include "../AppEngine/AppEngine.h"
#include "../DatabaseManager/DatabaseManager.h"
#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent final : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void resized() override;

    void showWelcomeView();
    void showTrackViewTutorial();
    void showInstrumentTutorial();

    void reportDatabaseSize();

private:
    std::unique_ptr<Component> view;
    AppEngine appEngine;

    DatabaseManager databaseManager;
};
