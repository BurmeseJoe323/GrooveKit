//
// Created by ikera on 4/6/2025.
//

#include "TrackComponent.h"

#include <tracktion_engine/utilities/tracktion_Identifiers.h>

using namespace juce;
TrackComponent::TrackComponent() {
    addAndMakeVisible(trackHeader);
    addAndMakeVisible(trackClip);
}

TrackComponent::~TrackComponent() {
    trackHeader.removeListener(this);
};

void TrackComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::grey);

    // this draws a line right after the track header
    // separating the clips from the track header
    if (trackHeader.getRight() > 0) {
        const int lineX = trackHeader.getRight() + 5.0f;
        g.setColour(juce::Colours::black);
        g.drawLine(lineX, 0, lineX, getHeight(), 2.0f);
    }
    // TODO: select tracks, Take a look at example from Tracktion
}

void TrackComponent::resized() {
    FlexBox mainFB;

    mainFB.flexDirection = FlexBox::Direction::row;
    mainFB.alignItems = FlexBox::AlignItems::stretch;
    auto area = getLocalBounds();

    //define the margins
    const float margin = 5.0f;
    const FlexItem::Margin itemMargin(margin);

    // Track Header
    mainFB.items.add(FlexItem(trackHeader)
        .withWidth(100)
        .withHeight(getHeight() - itemMargin.top - itemMargin.bottom)
        .withMargin(itemMargin));

    mainFB.items.add(FlexItem(trackClip)
        .withWidth(100)
        .withHeight(getHeight() - itemMargin.top - itemMargin.bottom)
        .withMargin(itemMargin));

    mainFB.performLayout(area);
}


void TrackComponent::onAddClipClicked() {
    // TODO: fix onAddClipclicked()
    DBG("clicked add clip");
}



