#include "TrackComponent.h"
#include "TrackListComponent.h"

TrackComponent::TrackComponent (const std::shared_ptr<AppEngine>& engine, const int trackIndex, const juce::Colour color)
    : appEngine (engine), trackColor (color), trackIndex (trackIndex)
{
    engineIndex = trackIndex;
    trackClip.setColor (trackColor);

    // Check if the track already has clips when created
    if (auto* track = appEngine->getTrackManager().getTrack (engineIndex))
    {
        if (track->getClips().size() > 0)
        {
            addAndMakeVisible (trackClip);
            numClips = track->getClips().size();
        }
    }
}

TrackComponent::~TrackComponent() = default;

void TrackComponent::paint (juce::Graphics& g)
{
    g.fillAll (trackColor.darker (0.4f));
    g.setColour (juce::Colours::black.withAlpha (0.3f));
    g.drawRect (getLocalBounds(), 1.0f);
}

void TrackComponent::resized()
{
    // juce::FlexBox mainFB;
    // mainFB.flexDirection = juce::FlexBox::Direction::row;
    // mainFB.alignItems = juce::FlexBox::AlignItems::stretch;
    //
    // const juce::FlexItem::Margin itemMargin (5.0f);
    // mainFB.items.add (juce::FlexItem (trackClip)
    //         .withWidth (125)
    //         .withHeight (getHeight() - itemMargin.top - itemMargin.bottom)
    //         .withMargin (itemMargin));
    //
    // mainFB.performLayout (getLocalBounds());

    // Set the clip's bounds directly, with a 5-pixel margin
    trackClip.setBounds (getLocalBounds().reduced (5).withWidth (150));
}

void TrackComponent::onSettingsClicked()
{
    juce::PopupMenu m;
    m.addItem (1, "Add MIDI Clip");
    m.addItem (2, "Open Editor");
    m.addSeparator();
    m.addItem (3, "Delete Track");

    m.showMenuAsync ({}, [this] (int result) {
        switch (result)
        {
            case 1: // Add Clip
                appEngine->addMidiClipToTrack (engineIndex);
                addAndMakeVisible (trackClip);
                resized();
                numClips = 1;
                break;
            case 2: // Open Editor
            {
                bool isDrumTrack = appEngine->isDrumTrack (engineIndex);
                if (isDrumTrack)
                {
                    if (onRequestOpenDrumSampler)
                        onRequestOpenDrumSampler (trackIndex);
                }
                else
                {
                    if (onRequestOpenPianoRoll && numClips > 0)
                        onRequestOpenPianoRoll (trackIndex);
                }
            }
            break;
            case 3: // Delete Track
                if (onRequestDeleteTrack)
                    onRequestDeleteTrack (trackIndex);
                break;
            default:
                break;
        }
    });
}

// void TrackComponent::onAddClipClicked()
// {
//     DBG ("Add clip -> UI " << trackIndex << " -> engine " << engineIndex);
//
//     appEngine->addMidiClipToTrack (engineIndex);
//     addAndMakeVisible (trackClip);
//     resized();
//     numClips = 1;
// }
//
// void TrackComponent::onDeleteTrackClicked()
// {
//     DBG ("Delete clicked for track index: " << trackIndex);
//     if (onRequestDeleteTrack)
//         onRequestDeleteTrack (trackIndex);
//     // deleteAllChildren();
// }
//
// void TrackComponent::onPianoRollClicked()
// {
//     DBG ("Piano Roll clicked for track index: " << trackIndex);
//     if (onRequestOpenPianoRoll && numClips > 0)
//         onRequestOpenPianoRoll (trackIndex);
// }
//
// void TrackComponent::onDrumSamplerClicked()
// {
//     DBG ("Drum Sampler clicked for track index: " << trackIndex);
//     if (onRequestOpenDrumSampler)
//         onRequestOpenDrumSampler (trackIndex);
// }

void TrackComponent::setTrackIndex (int index)
{
    this->trackIndex = index;
}

void TrackComponent::onMuteToggled (bool isMuted)
{
    if (appEngine)
        appEngine->setTrackMuted (engineIndex, isMuted);
}

void TrackComponent::onSoloToggled (bool isSolo)
{
    if (appEngine)
        appEngine->setTrackSoloed (engineIndex, isSolo);
    if (auto* p = findParentComponentOfClass<TrackListComponent>())
        p->refreshSoloVisuals();
}
