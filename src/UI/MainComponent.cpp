#ifdef _WIN32
#include <corecrt_io.h>
#endif


#include "MainComponent.h"
#include "MainViews/AppView.h"
#include "MainViews/WelcomeView.h"
#include "TrackView-UI/TrackView.h"
#include "InstrumentTutorial/InstrumentTutorial.h"


MainComponent::MainComponent()
{
    engine = std::make_unique<AppEngine>();
    view = std::make_unique<WelcomeView>();
    addAndMakeVisible(view.get());
    // view->setBounds(getLocalBounds());

    databaseManager.initialize();
    setSize(600, 400);
}

MainComponent::~MainComponent() = default;

void MainComponent::showWelcomeView() {
    removeAllChildren();
    view = std::make_unique<WelcomeView>();
    addAndMakeVisible(view.get());
    view->setBounds(getLocalBounds());
}

void MainComponent::showTrackView()
{
    removeAllChildren();
    databaseManager.addTutorial("TrackViewTutorial");
    view = std::make_unique<TrackView>(*engine);
    addAndMakeVisible(view.get());
    view->setBounds(getLocalBounds());
}

void MainComponent::showTrackViewTutorial()
{
    removeAllChildren();
    view = std::make_unique<TrackViewTut>(databaseManager);
    addAndMakeVisible(view.get());

    view->setBounds(getLocalBounds());
    // openTrackView.setVisible(false);
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



