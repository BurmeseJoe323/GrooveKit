#ifdef _WIN32
#include <corecrt_io.h>
#endif


#include "TrackView-UI/TrackEditView.h"
#include "MainComponent.h"
#include "MainViews/AppView.h"
#include "MainViews/WelcomeView.h"

MainComponent::MainComponent()
{
    setSize(600, 400);

    view = std::make_unique<WelcomeView>();
    addAndMakeVisible(view.get());
    // view->setBounds(getLocalBounds());

    databaseManager.initialize();
    setSize(600, 400);
}

MainComponent::~MainComponent() = default;

void MainComponent::showWelcomeView()
{
    removeAllChildren();
    view = std::make_unique<WelcomeView>();
    addAndMakeVisible(view.get());
    view->setBounds(getLocalBounds());
}

void MainComponent::showTrackView()
{
    removeAllChildren();
    view = std::make_unique<TrackEditView>();
    addAndMakeVisible(view.get());

    view->setBounds(getLocalBounds());
}

void MainComponent::showTrackViewTutorial()
{
    removeAllChildren();
    databaseManager.addTutorial("TrackViewTutorial");
    view = std::make_unique<TrackViewTut>(databaseManager);
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

void MainComponent::reportDatabaseSize()
{
    int size = databaseManager.selectCompletedTutorials("User1").size();
    std::cout << "There are: " << size <<  " elements in the array" << std::endl;
}



