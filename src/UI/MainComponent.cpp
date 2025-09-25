#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(1200, 800);
    showTrackView();
}

MainComponent::~MainComponent() = default;

void MainComponent::resized()
{
    if (view)
        view->setBounds(getLocalBounds());
}

void MainComponent::setView(std::unique_ptr<juce::Component> newView)
{
    if (view)
        removeChildComponent(view.get());

    view = std::move(newView);
    addAndMakeVisible(view.get());
    view->setBounds(getLocalBounds());
}

void MainComponent::showTrackView()
{
    auto tev = std::make_unique<TrackEditView>(appEngine);
    tev->onOpenMix = [this] { showMixView(); };
    setView(std::move(tev));
}

void MainComponent::showMixView()
{
    auto mv = std::make_unique<MixView>(appEngine);
    mv->onBack = [this] { showTrackView(); };
    setView(std::move(mv));
}
