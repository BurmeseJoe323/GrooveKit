#include "TrackComponent.h"
#include "TrackListComponent.h"

TrackComponent::TrackComponent (const std::shared_ptr<AppEngine>& engine, const int trackIndex, const juce::Colour color)
    : appEngine (engine), trackColor (color), trackIndex (trackIndex)
{
    trackClip.setColor (trackColor);

    // Check if the track already has clips when created.
    if (const auto* track = appEngine->getTrackManager().getTrack (trackIndex))
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
    const auto bounds = getLocalBounds().reduced (5);

    // If we can find a clip on this track, size/position the UI clip from its time range.
    if (const auto* track = appEngine ? appEngine->getTrackManager().getTrack (trackIndex) : nullptr)
    {
        if (track->getClips().size() > 0)
        {
            trackClip.setBounds (getLocalBounds().reduced (5).withWidth (280));
            return;
        }
    }
    trackClip.setBounds (bounds.withWidth (juce::jmax (bounds.getHeight(), 40)));
}

void TrackComponent::onSettingsClicked()
{
    juce::PopupMenu m;
    const bool isDrumTrack = appEngine->isDrumTrack (trackIndex);
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

    m.showMenuAsync ({}, [this] (const int result) {
        switch (result)
        {
            case 1: // Add Clip
                appEngine->addMidiClipToTrack (trackIndex);
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
            case 100: // Delete Track
                if (onRequestDeleteTrack)
                    onRequestDeleteTrack (trackIndex);
                break;
            default:
                break;
        }
    });
}

void TrackComponent::setTrackIndex (const int index)
{
    this->trackIndex = index;
}

int TrackComponent::getTrackIndex() const
{
    return trackIndex;
}

void TrackComponent::onMuteToggled (const bool isMuted)
{
    if (appEngine)
        appEngine->setTrackMuted (trackIndex, isMuted);
}

void TrackComponent::onSoloToggled (const bool isSolo)
{
    if (appEngine)
        appEngine->setTrackSoloed (trackIndex, isSolo);
    if (auto* p = findParentComponentOfClass<TrackListComponent>())
        p->refreshSoloVisuals();
}
