#include "WelcomeView.h"
#include "../MainComponent.h"
#include "TrackView/TrackEditView.h"
#include "TutorialManager/TutorialManagerComponent.h"

WelcomeView::WelcomeView (AppEngine& engine, DatabaseManager& db)
    : appEngine (engine), databaseManager (db)
{
    setSize (800, 600);

    addAndMakeVisible (openTrackView);
    addAndMakeVisible (openTutorialManager);
    addAndMakeVisible (deleteUserData);

    openTrackView.onClick = [this]() {
        auto* trackView = new TrackEditView();
        trackView->onBack = [this]() {
            currentView.reset();
            resized();
        };
        currentView.reset (trackView);
        addAndMakeVisible (currentView.get());
        resized();
    };

    openTutorialManager.onClick = [this]() {
        auto* tutorialManager = new TutorialManagerComponent (appEngine, databaseManager);
        tutorialManager->onBack = [this]() {
            currentView.reset();
            resized();
        };
        currentView.reset (tutorialManager);
        addAndMakeVisible (currentView.get());
        resized();
    };

    deleteUserData.onClick = [this]() {
        databaseManager.deleteUserData ("User1");
    };
}

void WelcomeView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawText ("Welcome to GrooveKit!", getLocalBounds().removeFromTop (40), juce::Justification::centred, true);
}

void WelcomeView::resized()
{
    auto area = getLocalBounds().reduced (20);
    openTrackView.setBounds (area.removeFromBottom (40).removeFromLeft (200));
    openTutorialManager.setBounds (area.removeFromBottom (40).removeFromLeft (200));
    deleteUserData.setBounds (area.removeFromBottom (40).removeFromLeft (200));

    if (currentView)
        currentView->setBounds (getLocalBounds().reduced (10));
}
