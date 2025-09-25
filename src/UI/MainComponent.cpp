#include "MainComponent.h"
#include "TrackView/TrackEditView.h"

MainComponent::MainComponent()
{
    setSize(1200, 800);
    view = std::make_unique<TrackEditView> (appEngine);
    addAndMakeVisible (view.get());
    view->setBounds (getLocalBounds());
}

MainComponent::~MainComponent() = default;

void MainComponent::resized()
{
    if (view)
        view->setBounds(getLocalBounds());
}
