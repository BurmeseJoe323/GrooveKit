//
// Created by Joseph Rockwell on 4/8/25.
//

#include "WelcomeView.h"

#include "AppView.h"
#include "../MainComponent.h"

WelcomeView::WelcomeView()
{
    setSize(600, 400);

    openAppView.onClick = [this]() {
        auto parent = dynamic_cast<MainComponent*>(getParentComponent());
        parent->showAppView();
    };
    addAndMakeVisible(openAppView);
}

WelcomeView::~WelcomeView() = default;

void WelcomeView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("Hello, GrooveKit!", getLocalBounds(), juce::Justification::centred, true);
}


void WelcomeView::resized()
{
    openAppView.setBounds(getWidth() - 176, getHeight() - 60, 120, 32);
    // layout child components here
}
