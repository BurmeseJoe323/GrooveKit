#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "MixView.h"

MixView::MixView(AppEngine& engine)
    : appEngine(engine)
{
    setOpaque(true);

    addAndMakeVisible(backButton);
    backButton.setButtonText("Track View");
    backButton.onClick = [this]{ if (onBack) onBack(); };

    mixerPanel = std::make_unique<MixerPanel>(appEngine);
    addAndMakeVisible(*mixerPanel);
}

MixView::~MixView() {
    mixerPanel.reset();
}


void MixView::paint (juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF495057));
}

void MixView::resized()
{
    auto bounds = getLocalBounds().reduced(outerMargin);

    auto topRow = bounds.removeFromTop(topBarHeight);

    const int btnW = 140, btnH = 36;
    backButton.setBounds(topRow.removeFromRight(btnW).withY(topRow.getY() + (topBarHeight - btnH) / 2)
                                        .withHeight(btnH).withWidth(btnW));

    mixerPanel->setBounds(bounds);
}