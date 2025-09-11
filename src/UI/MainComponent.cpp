#ifdef _WIN32
#include <corecrt_io.h>
#endif


#include "TrackView/TrackEditView.h"
#include "MainComponent.h"
#include "InstrumentTutorial/InstrumentTutorial.h"
#include "MainViews/WelcomeView.h"
#include "TrackEditViewTutorial/TrackEditViewTutorial.h"
#include "TrackView/TrackEditView.h"

MainComponent::MainComponent()
{
    setSize(1200, 800);
    databaseManager.initialize();

    view = std::make_unique<WelcomeView> (appEngine, databaseManager);
    addAndMakeVisible (view.get());
    view->setBounds (getLocalBounds());
}

MainComponent::~MainComponent() = default;

void MainComponent::showWelcomeView()
{
    removeAllChildren();
    view = std::make_unique<WelcomeView>(appEngine, databaseManager);
    addAndMakeVisible(view.get());
    view->setBounds(getLocalBounds());
}



void MainComponent::showTrackViewTutorial()
{
    removeAllChildren();
    databaseManager.addTutorial("TrackViewTutorial");
    view = std::make_unique<TrackEditViewTutorial>(databaseManager);
    addAndMakeVisible(view.get());

    view->setBounds(getLocalBounds());
}

void MainComponent::showInstrumentTutorial() {
    removeAllChildren();
    databaseManager.addTutorial("InstrumentTutorial");
    view = std::make_unique<InstrumentTutorial>(databaseManager);
    addAndMakeVisible(view.get());

    view->setBounds(getLocalBounds());
}

void MainComponent::resized()
{
    if (view)
        view->setBounds(getLocalBounds());
}

void MainComponent::reportDatabaseSize()
{
    int size = databaseManager.selectCompletedTutorials("User1").size();
    std::cout << "There are: " << size <<  " elements in the array" << std::endl;
}
