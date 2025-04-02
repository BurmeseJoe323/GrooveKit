#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(600, 400);
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
    // layout child components here
}
