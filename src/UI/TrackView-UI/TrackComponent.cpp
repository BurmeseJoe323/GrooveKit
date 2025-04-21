//
// Created by ikera on 4/6/2025.
//

#include "TrackComponent.h"

using namespace juce;
TrackComponent::TrackComponent(std::shared_ptr<AppEngine> engine, int index) : appEngine(engine), trackIndex(index){
}

TrackComponent::~TrackComponent() = default;

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
    DBG("Add Clip clicked for track index: " << trackIndex);
    appEngine->addMidiClipToTrack(trackIndex);
    addAndMakeVisible(trackClip);
    numClips = 1;
}

void TrackComponent::onDeleteTrackClicked() {
    DBG("Delete clicked for track index: " << trackIndex);
    if (onRequestDeleteTrack)
        onRequestDeleteTrack(trackIndex);
    // deleteAllChildren();
}

void TrackComponent::onPianoRollClicked() {
    DBG("Piano Roll clicked for track index: " << trackIndex);
    if (onRequestOpenPianoRoll && numClips > 0)
        onRequestOpenPianoRoll(trackIndex);
}


