//
// Created by ikera on 4/6/2025.
//

#include "TrackComponent.h"

#include <tracktion_engine/utilities/tracktion_Identifiers.h>

using namespace juce;
TrackComponent::TrackComponent() {
    addAndMakeVisible(trackClip);
}

TrackComponent::~TrackComponent() {
};

void TrackComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::grey);
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

    mainFB.items.add(FlexItem(trackClip)
        .withWidth(300)
        .withHeight(getHeight() - itemMargin.top - itemMargin.bottom)
        .withMargin(itemMargin));

    mainFB.performLayout(area);
}


void TrackComponent::onAddClipClicked() {
    // TODO: fix onAddClipclicked()
    DBG("clicked add clip");
}




