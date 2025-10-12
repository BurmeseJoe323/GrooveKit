#include "TrackComponent.h"
#include "TrackListComponent.h"

TrackComponent::TrackComponent (const std::shared_ptr<AppEngine>& engine, const int trackIndex, const juce::Colour color)
    : appEngine (engine), trackColor (color), trackIndex (trackIndex)
{
    if (appEngine)
        appEngine->registerTrackListener (trackIndex, this);

    // Determine clip pointer up-front
    te::MidiClip* midiClip = appEngine ? appEngine->getMidiClipFromTrack (trackIndex) : nullptr;
    // Construct TrackClip if we have a valid MidiClip
    if (midiClip != nullptr)
        trackClip = std::make_unique<TrackClip> (midiClip, 100.0f /* pixels per beat */);

    // If no color provided, pick one from a palette based on index
    if (!trackColor.isOpaque())
    {
        static const juce::Colour palette[] = {
            juce::Colour (0xff6fa8dc), // blue
            juce::Colour (0xff93c47d), // green
            juce::Colour (0xfff6b26b), // orange
            juce::Colour (0xffe06666), // red
            juce::Colour (0xff8e7cc3), // purple
            juce::Colour (0xff76a5af), // teal
            juce::Colour (0xffffd966) // yellow
        };
        trackColor = palette[std::abs (trackIndex) % (int) (sizeof (palette) / sizeof (palette[0]))];
    }

    // Check if the track already has clips when created.
    if (const auto* track = appEngine ? appEngine->getTrackManager().getTrack (trackIndex) : nullptr)
    {
        if (track->getClips().size() > 0 && trackClip)
        {
            addAndMakeVisible (trackClip.get());
            trackClip->setColor (trackColor);
            numClips = track->getClips().size();
        }
    }
}

TrackComponent::~TrackComponent()
{
    if (appEngine)
        appEngine->unregisterTrackListener (trackIndex, this);
}

void TrackComponent::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    const float radius = 10.0f;

    // Background
    g.setColour (trackColor.darker (0.4f));
    g.fillRoundedRectangle (r, radius);

    // Rounded border
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawRoundedRectangle (r, radius, 1.0f);
}

void TrackComponent::resized()
{
    const auto bounds = getLocalBounds().reduced (5);

    // If we can find a clip on this track, size/position the UI clip from its time range.
    if (const auto* track = appEngine ? appEngine->getTrackManager().getTrack (trackIndex) : nullptr)
    {
        if (track->getClips().size() > 0 && trackClip)
        {
            if (trackClip->getParentComponent() != this)
                addAndMakeVisible (trackClip.get());

            const int w = juce::jmax (1, trackClip->getWidth());
            trackClip->setBounds (bounds.withWidth (w));
            return;
        }
    }
    // Otherwise, set default size if we have a clip component
    if (trackClip)
        trackClip->setBounds (bounds.withWidth (juce::jmax (bounds.getHeight(), 40)));
}

// TrackComponent.cpp
void TrackComponent::onInstrumentClicked()
{
    if (appEngine)
        appEngine->openInstrumentEditor (trackIndex);
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
                if (!trackClip)
                {
                    if (auto* midiClip = appEngine->getMidiClipFromTrack (trackIndex))
                        trackClip = std::make_unique<TrackClip> (midiClip, 100.0f);
                }
                if (trackClip)
                {
                    addAndMakeVisible (trackClip.get());
                    trackClip->setColor (trackColor);
                }
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
    if (appEngine)
        appEngine->unregisterTrackListener (trackIndex, this);
    this->trackIndex = index;
    if (appEngine)
        appEngine->registerTrackListener (trackIndex, this);
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
        p->refreshTrackStates();
}

void TrackComponent::onRecordArmToggled (bool isArmed)
{
    if (auto* p = findParentComponentOfClass<TrackListComponent>())
        p->armTrack(trackIndex, isArmed);
}
