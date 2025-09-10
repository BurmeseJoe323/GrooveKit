//
// Created by Joseph Rockwell on 4/8/25.
//

#include "WelcomeView.h"

#include "../MainComponent.h"
#include "../TrackView/TrackEditView.h"
#include "../MixView/MixView.h"

WelcomeView::WelcomeView(AppEngine& engine, DatabaseManager& db)
    : appEngine(engine), databaseManager(db)
{
    setSize(1000, 1000);

    // Menu buttons
    addAndMakeVisible(openTrackView);
    addAndMakeVisible(openTutorialManager);
    addAndMakeVisible(deleteUserData);

    // ===== Persistent Track View =====
    trackView = std::make_unique<TrackEditView>(appEngine);
    addAndMakeVisible(*trackView);
    trackView->setVisible(false);

    trackView->onBack = [this]
    {
        // Hide the track view and return to the menu
        trackView->setVisible(false);
        resized();
    };

    trackView->onOpenMix = [this]
    {
        // Switch to Mix View
        if (mixView) mixView->refreshMixer();
        trackView->setVisible(false);
        if (mixView) mixView->setVisible(true);
        resized();
    };

    // ===== Persistent Mix View =====
    mixView = std::make_unique<MixView>(appEngine);
    addAndMakeVisible(*mixView);
    mixView->setVisible(false);

    mixView->onBack = [this]
    {
        // Back to Track View
        mixView->setVisible(false);
        if (trackView) trackView->setVisible(true);
        resized();
    };

    // ===== Button handlers =====
    openTrackView.onClick = [this]
    {
        // Open Track View from menu
        if (mixView) mixView->setVisible(false);
        if (trackView) trackView->setVisible(true);
        resized();
    };

    openTutorialManager.onClick = [this]
    {
        if (!tutorialManager)
        {
            tutorialManager = std::make_unique<TutorialManagerComponent>(appEngine, databaseManager);
            tutorialManager->onBack = [this]
            {
                removeChildComponent(tutorialManager.get());
                tutorialManager.reset(); // auto-deletes safely
                resized();
            };
            addAndMakeVisible(*tutorialManager);
        }

        tutorialManager->setBounds(getLocalBounds());
        tutorialManager->setVisible(true);
        resized();
    };


    deleteUserData.onClick = [this]
    {
        databaseManager.deleteUserData("User1");
    };
}

void WelcomeView::showTrack()
{
    if (!trackView)
    {
        trackView = std::make_unique<TrackEditView>(appEngine);
        trackView->onBack    = [this]{ trackView->setVisible(false); resized(); };
        trackView->onOpenMix = [this]{ showMix(); };
        addAndMakeVisible(*trackView);
    }
    if (mixView) mixView->setVisible(false);
    trackView->setVisible(true);
    resized();
}

void WelcomeView::showMix()
{
    if (!mixView)
    {
        mixView = std::make_unique<MixView>(appEngine);
        mixView->onBack = [this]{ showTrack(); };
        addAndMakeVisible(*mixView);
    }
    // always refresh from the current Edit when showing
    mixView->refreshMixer();
    if (trackView) trackView->setVisible(false);
    mixView->setVisible(true);
    resized();
}

void WelcomeView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("Welcome to GrooveKit!", getLocalBounds().removeFromTop(40), juce::Justification::centred, true);
}

void WelcomeView::resized()
{
    auto full = getLocalBounds();
    const bool showingTrack = trackView && trackView->isVisible();
    const bool showingMix   = mixView   && mixView->isVisible();

    if (showingTrack || showingMix)
    {
        // Hide menu when a child view is showing
        openTrackView.setVisible(false);
        openTutorialManager.setVisible(false);
        deleteUserData.setVisible(false);

        if (showingTrack) trackView->setBounds(full);
        if (showingMix)   mixView  ->setBounds(full);
        return;
    }

    // Menu layout
    openTrackView.setVisible(true);
    openTutorialManager.setVisible(true);
    deleteUserData.setVisible(true);

    auto area = full.reduced(20);
    openTrackView.setBounds(area.removeFromBottom(40).removeFromLeft(200));
    openTutorialManager.setBounds(area.removeFromBottom(40).removeFromLeft(200));
    deleteUserData.setBounds(area.removeFromBottom(40).removeFromLeft(200));
}
