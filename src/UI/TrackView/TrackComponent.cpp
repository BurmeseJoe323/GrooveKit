#include "TrackComponent.h"

#include "../../AppEngine/AppEngine.h"
#include "TrackEditView.h"
#include "TrackListComponent.h"

TrackComponent::TrackComponent (const std::shared_ptr<AppEngine>& engine, const int trackIndex, const juce::Colour color)
    : appEngine (engine), trackColor (color), trackIndex (trackIndex)
{
    if (appEngine)
        appEngine->registerTrackListener (trackIndex, this);
    // If no color provided, pick one from a palette based on index
    if (! trackColor.isOpaque())
    {
        static const juce::Colour palette[] = {
            juce::Colour (0xff6fa8dc), // blue
            juce::Colour (0xff93c47d), // green
            juce::Colour (0xfff6b26b), // orange
            juce::Colour (0xffe06666), // red
            juce::Colour (0xff8e7cc3), // purple
            juce::Colour (0xff76a5af), // teal
            juce::Colour (0xffffd966)  // yellow
        };
        trackColor = palette[std::abs(trackIndex) % (int) (sizeof(palette) / sizeof(palette[0]))];
    }


    rebuildFromEdit();
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
    const auto bounds = getLocalBounds().reduced(5);

    if (!appEngine)
        return;

    auto& trackManager = appEngine->getTrackManager();

    // Access the timeline scaling info through the parent TrackListComponent
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    const double pixelsPerSecond = tl ? tl->getPixelsPerSecond() : 100.0;
    const double viewStartSec    = tl ? tl->getViewStart().inSeconds() : 0.0;

    const double pps      = pixelsPerSecond;
    const double viewS    = viewStartSec;

    auto& edit =
#ifdef HAS_APPENGINE_POINTER
    appEngine->getEdit();
    #else
        appEngine->getEdit();
    #endif
    for (auto* ui : clipUIs)
    {
        auto* midiClip = ui->getClip();
        if (!midiClip) continue;

        const auto posRange = midiClip->getPosition().time;

        te::TimeRange drawRange = posRange;
        if (midiClip->isLooping())
        {
            const auto loopRange = midiClip->getLoopRange();
            drawRange = te::TimeRange(posRange.getStart(), posRange.getStart() + loopRange.getLength());
        }

        const double clipStart = drawRange.getStart().inSeconds();
        const double clipLen   = drawRange.getLength().inSeconds();

        const int x = (int) juce::roundToIntAccurate((clipStart - viewStartSec) * pixelsPerSecond);
        const int w = (int) juce::roundToIntAccurate(clipLen * pixelsPerSecond);

        ui->setBounds(x, bounds.getY(), juce::jmax(w, 20), bounds.getHeight());
    }

}

void TrackComponent::rebuildFromEdit()
{
    clipUIs.clear (true);

    if (!appEngine) return;

    auto& edit = appEngine->getEdit();
    auto audioTracks = te::getAudioTracks(edit);
    if (!juce::isPositiveAndBelow(trackIndex, audioTracks.size()))
        return;

    if (auto* teTrack = audioTracks.getUnchecked(trackIndex))
    {
        auto clips = teTrack->getClips();
        for (auto* c : clips)
        {
            if (auto* mc = dynamic_cast<te::MidiClip*>(c))
            {
                auto* ui = new TrackClip();
                ui->setClip(mc);
                ui->setColor (trackColor);
                ui->onOpen = [this](te::MidiClip* clicked)
                {
                    if (!clicked) return;
                    if (auto* parent = findParentComponentOfClass<TrackEditView>())
                        parent->showPianoRoll(clicked);
                };

                addAndMakeVisible(ui);
                clipUIs.add(ui);
            }
        }
    }

    resized();

    if (auto* list = findParentComponentOfClass<TrackListComponent>())
    {
        int rightmost = 0;
        // TrackComponent X is the left offset inside TrackList
        const int trackLeftInList = getX();

        for (auto* ui : clipUIs)
            if (ui)
                rightmost = std::max(rightmost, trackLeftInList + ui->getRight());

        // Account for the header column (same value used in TrackListComponent)
        constexpr int headerWidth = 140;
        const int requiredWidth = headerWidth + std::max(rightmost, getParentWidth() - headerWidth);

        if (requiredWidth > list->getWidth())
            list->setSize(requiredWidth + 40 /* small pad */, list->getHeight());
    }
}


// TrackComponent.cpp
void TrackComponent::onInstrumentClicked()
{
    if (!appEngine)
        return;

    if (appEngine->isDrumTrack(trackIndex))
    {
        if (onRequestOpenDrumSampler)
            onRequestOpenDrumSampler(trackIndex);
        return;
    }

    appEngine->openInstrumentEditor(trackIndex);
}


void TrackComponent::onSettingsClicked()
{
    juce::PopupMenu m;
    const bool isDrumTrack = appEngine->isDrumTrack (trackIndex);
    m.addItem (1, "Add MIDI Clip");
    m.addSeparator();

    m.addSeparator();
    m.addItem (100, "Delete Track");

    m.showMenuAsync ({}, [this] (const int result) {
        switch (result)
        {
            case 1: // Add Clip
                appEngine->addMidiClipToTrack (trackIndex);
                rebuildFromEdit();
                resized();
                numClips = 1;
                break;
            case 10: // Open Drum Sampler
                if (onRequestOpenDrumSampler)
                    onRequestOpenDrumSampler (trackIndex);
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


