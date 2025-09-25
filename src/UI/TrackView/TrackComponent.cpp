#include "TrackComponent.h"
#include "TrackListComponent.h"
namespace te = tracktion;

TrackComponent::TrackComponent (AppEngine* engine,
                                te::AudioTrack* t,
                                juce::Colour color)
    : appEngine (engine),
      trackPtr  (t),
      trackColor (color)
{
    trackClip.setColor (trackColor);

    if (trackPtr != nullptr)
    {
        const auto& clips = trackPtr->getClips();
        if (clips.size() > 0)
        {
            addAndMakeVisible (trackClip);
            numClips = (int) clips.size();
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

    if (trackPtr != nullptr)
    {
        if (trackPtr->getClips().size() > 0)
        {
            trackClip.setBounds (getLocalBounds().reduced (5).withWidth (280));
            return;
        }
    }
    trackClip.setBounds (bounds.withWidth (juce::jmax (bounds.getHeight(), 40)));
}

int TrackComponent::getTrackIndex () const
{
    return appEngine ? appEngine->getTrackManager().indexOfTrack (trackPtr) : -1;
}

void TrackComponent::setPixelsPerSecond (double pps)
{
    if (pps <= 0.0) pps = 1.0;
    if (pixelsPerSecond == pps) return;
    pixelsPerSecond = pps;
    resized();      // layout may depend on pps
    repaint();
}

void TrackComponent::setViewStart (te::TimePosition start)
{
    // if (viewStart == start) return; // ok to skip compare if your TE build lacks == on TimePosition
    viewStart = start;
    resized();      // layout may depend on view origin
    repaint();
}

void TrackComponent::onInstrumentPressed (juce::Component& anchor)
{
    if (!appEngine) return;

    const int engineIdx = getTrackIndex();

    // Drum track? open your custom UI
    if (appEngine->isDrumTrack (engineIdx))
    {
        if (onRequestOpenDrumSampler) onRequestOpenDrumSampler (uiIndex);
        return;
    }

    // Otherwise open the plugin editor, or show chooser if none
    if (trackPtr != nullptr)
    {
        if (auto* inst = appEngine->findInstrument (*trackPtr))
        {
            appEngine->openPluginWindow (*inst, &anchor);
            return;
        }
        appEngine->showInstrumentMenuForTrack (*trackPtr, anchor, {});
    }
}

void TrackComponent::onSettingsClicked()
{
    if (!appEngine) return;

    const int engineIdx = getTrackIndex();
    juce::PopupMenu m;
    const bool isDrum = appEngine->isDrumTrack (engineIdx);

    m.addItem (1, "Add MIDI Clip");
    m.addSeparator();
    if (isDrum) {
        m.addItem (10, "Open Drum Sampler");
        m.addItem (11, "Open Piano Roll");
    } else {
        m.addItem (11, "Open Piano Roll");
    }
    m.addSeparator();
    m.addItem (100, "Delete Track");

    m.showMenuAsync ({}, [this, engineIdx] (int result)
    {
        switch (result)
        {
            case 1:
                appEngine->addMidiClipToTrack (engineIdx);
                addAndMakeVisible (trackClip);
                resized();
                numClips = 1;
                break;
            case 10:
                if (onRequestOpenDrumSampler) onRequestOpenDrumSampler (uiIndex);
                break;
            case 11:
                if (onRequestOpenPianoRoll && numClips > 0) onRequestOpenPianoRoll (uiIndex);
                break;
            case 100:
                if (onRequestDeleteTrack) onRequestDeleteTrack (uiIndex);
                break;
            default: break;
        }
    });
}

void TrackComponent::onMuteToggled (bool isMuted)
{
    if (appEngine) appEngine->setTrackMuted (getTrackIndex(), isMuted);
}
void TrackComponent::onSoloToggled (bool isSolo)
{
    if (appEngine) appEngine->setTrackSoloed (getTrackIndex(), isSolo);
    if (auto* p = findParentComponentOfClass<TrackListComponent>())
        p->refreshSoloVisuals();
}
