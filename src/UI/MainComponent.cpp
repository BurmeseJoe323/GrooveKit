#include "MainComponent.h"

#ifdef _WIN32
#include <corecrt_io.h>
#endif

#include "TrackView.h"

MainComponent::MainComponent()
{
    setSize(600, 400);

    openTrackView.onClick = [this]() { showTrackView(); };
    addAndMakeVisible(openTrackView);
}

MainComponent::~MainComponent() = default;

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("Hello, GrooveKit!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    openTrackView.setBounds(getWidth() - 176, getHeight() - 60, 120, 32);
    // layout child components here
}

void MainComponent::showTrackView() {
    trackView = std::make_unique<TrackView>();
    addAndMakeVisible(trackView.get());
    trackView->setBounds(getLocalBounds());
    openTrackView.setVisible(false);
}

