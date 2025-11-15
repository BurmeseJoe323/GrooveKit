#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Create shared transport bar (Written by Claude Code)
    transportBar = std::make_unique<TransportBar>(appEngine);
    transportBar->onSwitchView = [this] {
        // Toggle between views based on current view mode
        if (dynamic_cast<TrackEditView*>(view.get()))
            showMixView();
        else
            showTrackView();
    };

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
    auto tev = std::make_unique<TrackEditView>(appEngine, *transportBar);
    tev->onOpenMix = [this] { showMixView(); };
    transportBar->setViewMode(TransportBar::ViewMode::TrackEdit);
    setView(std::move(tev));
}

void MainComponent::showMixView()
{
    auto mv = std::make_unique<MixView>(appEngine, *transportBar);
    mv->onBack = [this] { showTrackView(); };
    transportBar->setViewMode(TransportBar::ViewMode::Mix);
    setView(std::move(mv));
}
