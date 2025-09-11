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
    auto bounds = getLocalBounds().reduced (5);

    // If we can find a clip on this track, size/position the UI clip from its time range.
    if (auto* track = appEngine ? appEngine->getTrackManager().getTrack (engineIndex) : nullptr)
        {
            if (track->getClips().size() > 0)
            {
                auto* teClip = track->getClips().getUnchecked (0); // first clip in engine track
                auto  tr     = teClip->getPosition().time;          // te::TimeRange in seconds

                const int x0 = xFromTime (tr.getStart(), viewStart, pixelsPerSecond);
                const int x1 = xFromTime (tr.getEnd(),   viewStart, pixelsPerSecond);
                const int width = juce::jmax (1, x1 - x0);

                trackClip.setBounds (getLocalBounds().reduced (5).withWidth (280));
                return;
            }
        }


    trackClip.setBounds (bounds.withWidth (juce::jmax (bounds.getHeight(), 40)));
}

void TrackComponent::onSettingsClicked()
{
    juce::PopupMenu m;
    const bool isDrumTrack = appEngine->isDrumTrack (engineIndex);
    m.addItem (1, "Add MIDI Clip");
    m.addSeparator();
    if (isDrumTrack)
    {
        m.addItem (10, "Open Drum Sampler");
        m.addItem (11, "Open Piano Roll");
    }
    else
    {
        m.addItem (11, "Open Piano Roll");
    }
    m.addSeparator();
    m.addItem (100, "Delete Track");

    m.showMenuAsync ({}, [this] (int result) {
        switch (result)
        {
            case 1: // Add Clip
                appEngine->addMidiClipToTrack (engineIndex);
                addAndMakeVisible (trackClip);
                resized();
                numClips = 1;
                break;
            case 10: // Open Drum Sampler
                if (onRequestOpenDrumSampler)
                    onRequestOpenDrumSampler (trackIndex);
                break;
            case 11: // Open Piano Roll
                if (onRequestOpenPianoRoll && numClips > 0)
                    onRequestOpenPianoRoll (trackIndex);
                break;
                break;
            case 100: // Delete Track
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
