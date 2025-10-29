// JUNIE
#include "TrackComponent.h"

#include "TrackEditView.h"
#include "TrackListComponent.h"

TrackComponent::TrackComponent (const std::shared_ptr<AppEngine>& engine, const int trackIndex, const juce::Colour color)
    : appEngine (engine), trackColor (color), trackIndex (trackIndex)
{
    if (appEngine)
        appEngine->registerTrackListener (trackIndex, this);

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
        trackColor = palette[std::abs (trackIndex) % static_cast<int> (std::size (palette))];
    }

    if (appEngine)
        rebuildClipsFromEngine();
}

TrackComponent::~TrackComponent()
{
    if (appEngine)
        appEngine->unregisterTrackListener (trackIndex, this);
}

void TrackComponent::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat().reduced (1.0f);
    constexpr float radius = 10.0f;

    // Background
    g.setColour (trackColor.darker (0.4f));
    g.fillRoundedRectangle (r, radius);

    // Rounded border
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawRoundedRectangle (r, radius, 1.0f);
}

void TrackComponent::resized()
{
    const auto inner = getLocalBounds().reduced (5);

    // Layout all clip components based on their start beats
    int trackHeight = inner.getHeight();
    for (auto& uiClip : trackClips)
    {
        if (! uiClip)
            continue;
        // keep width consistent with current pixelsPerBeat setting
        uiClip->setPixelsPerBeat (pixelsPerBeat);

        int width = juce::jmax (1, uiClip->getWidth());

        int x = 0;
        if (auto* mc = uiClip->getMidiClip())
        {
            const double startBeats = mc->getStartBeat().inBeats();
            x = juce::roundToInt (startBeats * (double) pixelsPerBeat);
        }

        uiClip->setBounds (inner.getX() + x, inner.getY(), width, trackHeight);
    }
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
    m.addItem (2, "Paste at End");
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
            case 1: // Add Clip (Junie)
            {
                appEngine->addMidiClipToTrack (trackIndex);
                rebuildClipsFromEngine();
                break;
            }
            case 2:
            {
                if (appEngine)
                {
                    // Place at end of last clip
                    double startBeats = 0.0;
                    for (auto* mc : appEngine->getMidiClipsFromTrack (trackIndex))
                        startBeats = std::max (startBeats, mc->getStartBeat().inBeats() + mc->getLengthInBeats().inBeats());

                    if (appEngine->pasteClipboardAt (trackIndex, startBeats))
                        rebuildClipsFromEngine();
                }
                break;
            }
            case 10: // Open Drum Sampler
                if (onRequestOpenDrumSampler)
                    onRequestOpenDrumSampler (trackIndex);
                break;
            case 11: // Open Piano Roll
                if (onRequestOpenPianoRoll && ! trackClips.empty())
                    onRequestOpenPianoRoll (trackClips.front()->getMidiClip());
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

void TrackComponent::rebuildClipsFromEngine()
{
    // Remove existing UI clips
    trackClips.clear();

    if (! appEngine)
        return;

    auto clips = appEngine->getMidiClipsFromTrack (trackIndex);
    for (auto* mc : clips)
    {
        auto ui = std::make_unique<TrackClip> (mc, pixelsPerBeat);
        ui->setColor (trackColor);

        // Existing open piano roll callback
        ui->onClicked = [this] (te::MidiClip* c)
        {
            if (onRequestOpenPianoRoll)
                onRequestOpenPianoRoll (c);
        };

        // New: clipboard callbacks
        ui->onCopyRequested = [this] (te::MidiClip* c)
        {
            if (appEngine) appEngine->copyMidiClip (c);
        };

        ui->onDuplicateRequested = [this] (te::MidiClip* c)
        {
            if (appEngine) {
                appEngine->duplicateMidiClip (c);
                rebuildClipsFromEngine();
                resized();
            }
        };

        ui->onPasteRequested = [this] (te::MidiClip* c, double pasteBeats)
        {
            juce::ignoreUnused (c); // track determination is based on this component’s trackIndex
            if (appEngine) {
                appEngine->pasteClipboardAt (trackIndex, pasteBeats);
                rebuildClipsFromEngine();
                resized();
            }
        };

        ui->onDeleteRequested = [this] (te::MidiClip* c)
        {
            if (auto* parent = findParentComponentOfClass<TrackEditView>())
            {
                // If the piano roll is currently showing a clip from this track,
                // hide it before removing the clip to avoid dangling UI state.
                if (parent->getPianoRollIndex() == trackIndex)
                    parent->hidePianoRoll();
            }

            if (appEngine) {
                appEngine->deleteMidiClip (c);
                rebuildClipsFromEngine();
                resized();
            }
        };

        // Right-click context menu is owned by TrackComponent now
        ui->onContextMenuRequested = [this] (te::MidiClip* c, double pasteBeats)
        {
            if (c == nullptr)
                return;

            juce::PopupMenu m;
            m.addItem (1, "Copy");
            m.addItem (2, "Duplicate");
            m.addSeparator();
            m.addItem (3, "Delete");

            m.showMenuAsync ({}, [safeThis = juce::Component::SafePointer<TrackComponent>(this), clip = c] (int result)
            {
                if (safeThis == nullptr || safeThis->appEngine == nullptr)
                    return;

                switch (result)
                {
                    case 1: // Copy
                        safeThis->appEngine->copyMidiClip (clip);
                        break;
                    case 2: // Duplicate
                        safeThis->appEngine->duplicateMidiClip (clip);
                        safeThis->rebuildClipsFromEngine();
                        safeThis->resized();
                        break;
                    case 3: // Delete
                    {
                        if (auto* parent = safeThis->findParentComponentOfClass<TrackEditView>())
                        {
                            if (parent->getPianoRollIndex() == safeThis->trackIndex)
                                parent->hidePianoRoll();
                        }
                        safeThis->appEngine->deleteMidiClip (clip);
                        safeThis->rebuildClipsFromEngine();
                        safeThis->resized();
                        break;
                    }
                    default:
                        break;
                }
            });
        };

        addAndMakeVisible (ui.get());
        trackClips.push_back (std::move (ui));
    }

    resized();
}


void TrackComponent::mouseUp (const juce::MouseEvent& e)
{
    // Only handle background right-clicks on the TrackComponent itself
    if (! e.mods.isPopupMenu())
        return;

    // If the original component was a child (e.g., a TrackClip), let it handle its own menu
    if (e.originalComponent != this)
        return;

    // Compute beat position from mouse X within the inner bounds used for layout
    const auto inner = getLocalBounds().reduced (5);
    const int localX = juce::jmax (0, e.getPosition().x - inner.getX());
    const double beatPos = static_cast<double> (localX) / juce::jmax (1.0f, pixelsPerBeat);

    juce::PopupMenu m;
    m.addItem (1, "Paste Here");
    m.addItem (2, "Add MIDI Clip Here");

    m.showMenuAsync ({}, [safeThis = juce::Component::SafePointer<TrackComponent>(this), beatPos] (int result)
    {
        if (safeThis == nullptr || safeThis->appEngine == nullptr)
            return;

        switch (result)
        {
            case 1: // Paste Here
            {
                if (safeThis->appEngine->pasteClipboardAt (safeThis->trackIndex, beatPos))
                {
                    safeThis->rebuildClipsFromEngine();
                    safeThis->resized();
                }
                break;
            }
            case 2: // Add MIDI Clip Here
            {
                if (safeThis->appEngine->addMidiClipToTrackAt (safeThis->trackIndex, beatPos))
                {
                    safeThis->rebuildClipsFromEngine();
                    safeThis->resized();
                }
                break;
            }
            default: break;
        }
    });
}
