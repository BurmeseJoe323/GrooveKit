//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackView-UI/TrackView.h"
#include "../AppEngine/AppEngine.h"
#include "TrackView-Tutorial/TrackViewTut.h"
#include "../DatabaseManager/DatabaseManager.h"
#include "InstrumentTutorial/InstrumentTutorial.h"

class MainComponent : public juce::Component
{
public:
    MainComponent(AppEngine& engine);
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    TextButton openTrackView {"OPEN TRACKVIEW"};
    TextButton openTrackViewTut {"TrackView Tutorial"};
    TextButton openInstTutorial {"Instrument Tutorial"};
    TextButton selectCompletedTutorials {"See completed Tutorials"};
    std::unique_ptr<TrackView> trackView;
    std::unique_ptr<TrackViewTut> trackViewTut;
    std::unique_ptr<InstrumentTutorial> instTutorial;
    AppEngine& appEngine;
    DatabaseManager databaseManager;
    void showTrackView();
    void showTrackViewTutorial();
    void showInstrumentTutorial();
};
