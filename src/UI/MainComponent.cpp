#include "MainComponent.h"
#include "MainViews/AppView.h"
#include "MainViews/TrackView.h"
#include "MainViews/WelcomeView.h"

#ifdef _WIN32
#include <corecrt_io.h>
#endif


MainComponent::MainComponent() {
    view = std::make_unique<WelcomeView>();
    addAndMakeVisible(view.get());

    setSize(600, 400);
}

MainComponent::~MainComponent() = default;

void MainComponent::showTrackView() {
    removeAllChildren();
    view = std::make_unique<TrackView>();
    addAndMakeVisible(view.get());
    view->setBounds(getLocalBounds());
}

// layout child components here

