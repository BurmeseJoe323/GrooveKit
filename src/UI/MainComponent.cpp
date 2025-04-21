#include "MainComponent.h"

#ifdef _WIN32
#include <corecrt_io.h>
#endif


#include "TrackView-UI/TrackEditView.h"
#include "MainViews/WelcomeView.h"

MainComponent::MainComponent()
{
    setSize(600, 400);

    view = std::make_unique<WelcomeView>();
    addAndMakeVisible(view.get());
}

MainComponent::~MainComponent() = default;

void MainComponent::showTrackView()
{
    removeAllChildren();
    view = std::make_unique<TrackEditView>();
    addAndMakeVisible(view.get());
    view->setBounds(getLocalBounds());
}

    // layout child components here

