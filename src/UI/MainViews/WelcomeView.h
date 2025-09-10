//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H


#include <juce_gui_basics/juce_gui_basics.h>
#include "../../DatabaseManager/DatabaseManager.h"
#include "MixView/MixView.h"
#include "TrackView/TrackEditView.h"
#include "TutorialManager/TutorialManagerComponent.h"

class AppEngine;
using namespace juce;

class WelcomeView : public juce::Component
{
public:
    WelcomeView(AppEngine& engine, DatabaseManager& db);
    ~WelcomeView() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    std::unique_ptr<TrackEditView> trackView;
    std::unique_ptr<MixView>       mixView;
    std::unique_ptr<TutorialManagerComponent> tutorialManager;
    juce::TextButton openTrackView { "Open Track View" };
    juce::TextButton openTutorialManager { "Tutorials" };
    TextButton deleteUserData { "Delete User Data" };

    std::unique_ptr<juce::Component> currentView;
    AppEngine& appEngine;
    DatabaseManager& databaseManager;

    void showTrack();
    void showMix();
};



#endif //WELCOMEVIEW_H
