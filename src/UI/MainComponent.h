//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include "../AppEngine/AppEngine.h"
#include "../DatabaseManager/DatabaseManager.h"
#include "InstrumentTutorial/InstrumentTutorial.h"
#include "TrackEditViewTutorial/TrackEditViewTutorial.h"
#include "TrackView/TrackView.h"
#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent : public juce::Component
{
public:
    MainComponent(AppEngine& engine);
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    AppEngine& appEngine;
    DatabaseManager databaseManager;
    std::unique_ptr<TrackView> trackView;
    std::unique_ptr<TrackEditViewTutorial> trackViewTut;
    std::unique_ptr<InstrumentTutorial> instTutorial;
    std::unique_ptr<TutorialManagerComponent> tutorialManager;

    TextButton openTrackView {"TrackView"};
    TextButton openTrackViewTut {"TrackView Tutorial"};
    TextButton openInstTutorial {"Instrument Tutorial"};
    TextButton openTutorialManager {"Tutorials"};

    void showTrackView();
    void showTrackViewTutorial();
    void showInstrumentTutorial();
    void showTutorialManager();
};
