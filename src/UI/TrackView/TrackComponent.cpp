//
// Created by ikera on 4/6/2025.
//

#include "TrackComponent.h"

#include "TrackListComponent.h"

using namespace juce;
TrackComponent::TrackComponent(std::shared_ptr<AppEngine> engine, int index)
    : appEngine(engine), trackIndex(index)
{
    engineIndex = index; //
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
        .withWidth(125)
        .withHeight(getHeight() - itemMargin.top - itemMargin.bottom)
        .withMargin(itemMargin));

    mainFB.performLayout(area);
}


void TrackComponent::onAddClipClicked() {
    DBG("Add clip -> UI " << trackIndex << " -> engine " << engineIndex);

    appEngine->addMidiClipToTrack(engineIndex);
    addAndMakeVisible(trackClip);
    resized();
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

void TrackComponent::onDrumSamplerClicked() {
    DBG("Drum Sampler clicked for track index: " << trackIndex);
    if (onRequestOpenDrumSampler)
        onRequestOpenDrumSampler(trackIndex);
}

void TrackComponent::setTrackIndex(int index)
{
    this->trackIndex = index;
}

void TrackComponent::onMuteToggled(bool isMuted) {
    if (appEngine)
        appEngine->setTrackMuted(engineIndex, isMuted);
}

void TrackComponent::onSoloToggled(bool isSolo) {
    if (appEngine)
        appEngine->setTrackSoloed(engineIndex, isSolo);
    if (auto* p = findParentComponentOfClass<TrackListComponent>())
        p->refreshSoloVisuals();
}



