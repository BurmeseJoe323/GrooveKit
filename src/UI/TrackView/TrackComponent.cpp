#include "TrackComponent.h"
#include "TrackEditView.h"
#include "TrackListComponent.h"
#include <limits> // For std::numeric_limits (Written by Claude Code)

namespace t = tracktion;

TrackComponent::TrackComponent (const std::shared_ptr<AppEngine>& engine, const int trackIndex, const juce::Colour color)
    : appEngine (engine), trackColor (color), trackIndex (trackIndex)
{
    if (appEngine)
        appEngine->registerTrackListener (trackIndex, this);

    // If no color provided, pick one from a palette based on index
    if (!trackColor.isOpaque())
    {
        static const juce::Colour palette[] = {
            juce::Colour (0xffff6b6b),  // Red
            juce::Colour (0xfff06595),  // Pink
            juce::Colour (0xffcc5de8),  // Purple
            juce::Colour (0xff845ef7),  // Deep Purple
            juce::Colour (0xff5c7cfa),  // Indigo
            juce::Colour (0xff339af0),  // Blue
            juce::Colour (0xff22b8cf),  // Cyan
            juce::Colour (0xff20c997),  // Teal
            juce::Colour (0xff51cf66),  // Green
            juce::Colour (0xfffcc419)   // Yellow
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

    // Background - add red tint if this track is armed and recording
    auto bgColor = trackColor.darker (0.4f);

    if (appEngine && appEngine->isRecording() &&
        appEngine->getArmedTrackIndex() == trackIndex)
    {
        // Blend with red to indicate active recording on this track
        bgColor = bgColor.interpolatedWith(juce::Colours::darkred, 0.3f);
    }

    g.setColour (bgColor);
    g.fillRoundedRectangle (r, radius);

    // Draw recording preview clip if this track is being recorded to
    if (appEngine && appEngine->isRecording() &&
        appEngine->getArmedTrackIndex() == trackIndex)
    {
        auto previewBounds = appEngine->getRecordingPreviewBounds();
        if (!previewBounds.isEmpty())
        {
            // Get beat-based coordinates from parent TrackListComponent
            auto* tl = findParentComponentOfClass<TrackListComponent>();
            if (tl)
            {
                const double pixelsPerBeat = tl->getPixelsPerBeat();
                const double viewStartBeats = tl->getViewStartBeat().inBeats();
                auto& tempoSeq = appEngine->getEdit().tempoSequence;

                // Convert preview time range to beat positions
                const double clipStartBeats = tempoSeq.toBeats(previewBounds.getStart()).inBeats();
                const double clipEndBeats = tempoSeq.toBeats(previewBounds.getEnd()).inBeats();
                const double clipLenBeats = clipEndBeats - clipStartBeats;

                // Calculate preview clip position and size
                const int x = (int) juce::roundToIntAccurate((clipStartBeats - viewStartBeats) * pixelsPerBeat);
                const int w = (int) juce::roundToIntAccurate(clipLenBeats * pixelsPerBeat);

                // Draw semi-transparent preview clip
                const auto clipBounds = getLocalBounds().reduced(5);
                const auto previewRect = juce::Rectangle<int>(x, clipBounds.getY(),
                                                               juce::jmax(w, 2), clipBounds.getHeight());

                // Use track color with transparency for preview
                g.setColour(trackColor.withAlpha(0.4f));
                g.fillRoundedRectangle(previewRect.toFloat(), 8.0f);

                // Draw border around preview
                g.setColour(trackColor.brighter(0.3f).withAlpha(0.7f));
                g.drawRoundedRectangle(previewRect.toFloat(), 8.0f, 2.0f);
            }
        }
    }

    // Rounded border
    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawRoundedRectangle (r, radius, 1.0f);
}

void TrackComponent::resized()
{
    const auto bounds = getLocalBounds().reduced (5);

    if (!appEngine)
        return;

    // Access the beat-based scaling info through the parent TrackListComponent
    auto* tl = findParentComponentOfClass<TrackListComponent>();
    const double pixelsPerBeat = tl ? tl->getPixelsPerBeat() : 100.0;
    const double viewStartBeats = tl ? tl->getViewStartBeat().inBeats() : 0.0;

    auto& tempoSeq = appEngine->getEdit().tempoSequence;

    for (auto* ui : clipUIs)
    {
        auto* midiClip = ui->getMidiClip();
        if (!midiClip)
            continue;

        const auto posRange = midiClip->getPosition().time;
        t::TimeRange drawRange = posRange;

        if (midiClip->isLooping())
        {
            const auto loopRange = midiClip->getLoopRange();
            drawRange = t::TimeRange(posRange.getStart(), posRange.getStart() + loopRange.getLength());
        }

        // Convert time positions to beat positions for layout
        const double clipStartBeats = tempoSeq.toBeats(drawRange.getStart()).inBeats();
        const double clipEndBeats = tempoSeq.toBeats(drawRange.getEnd()).inBeats();
        const double clipLenBeats = clipEndBeats - clipStartBeats;

        const int x = (int) juce::roundToIntAccurate ((clipStartBeats - viewStartBeats) * pixelsPerBeat);
        const int w = (int) juce::roundToIntAccurate (clipLenBeats * pixelsPerBeat);

        ui->setBounds (x, bounds.getY(), juce::jmax (w, 20), bounds.getHeight());
    }
}

void TrackComponent::onInstrumentClicked()
{
    if (!appEngine)
        return;

    if (appEngine->isDrumTrack (trackIndex))
    {
        if (onRequestOpenDrumSampler)
            onRequestOpenDrumSampler (trackIndex);
        return;
    }

    appEngine->openInstrumentEditor (trackIndex);
}

void TrackComponent::onInstrumentMenuRequested()
{
    if (appEngine)
        appEngine->showInstrumentChooser (trackIndex);
}

void TrackComponent::onSettingsClicked()
{
    juce::PopupMenu m;
    m.addItem (1, "Add MIDI Clip");

    // Only show paste option if clipboard has compatible content (Junie)
    if (appEngine && appEngine->canPasteToTrack (trackIndex))
        m.addItem (2, "Paste at End");

    m.addSeparator();
    m.addItem (3, "Import MIDI Clip");
    m.addSeparator();

    m.addSeparator();
    m.addItem (100, "Delete Track");

    m.showMenuAsync ({}, [this] (const int result) {
        switch (result)
        {
            case 1: // Add Clip
            {
                appEngine->addMidiClipToTrack (trackIndex);
                rebuildAndRefreshHighlight();
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
                        rebuildAndRefreshHighlight();
                }
                break;
            }
            case 3: // Import MIDI Clip
            {
                if (appEngine)
                {
                    auto destStart = t::TimePosition::fromSeconds (0.0);

                    auto midiClips = appEngine->getMidiClipsFromTrack (trackIndex);

                    for (auto* mc : midiClips)
                    {
                        auto clipEnd = mc->getPosition().getEnd();
                        if (clipEnd > destStart)
                            destStart = clipEnd;
                    }

                    appEngine->importMidiClipViaChooser (
                        trackIndex,
                        destStart,
                        [this]()
                        {
                            rebuildAndRefreshHighlight();
                        });
                }
                break;
            }
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
        p->armTrack (trackIndex, isArmed);
}

void TrackComponent::rebuildClipsFromEngine()
{
    // Remove existing UI clips
    clipUIs.clear();

    if (!appEngine)
        return;

    auto clips = appEngine->getMidiClipsFromTrack (trackIndex);
    for (auto* mc : clips)
    {
        auto ui = std::make_unique<TrackClip> (mc, pixelsPerBeat);
        ui->setColor (trackColor);

        // Existing open piano roll callback
        ui->onClicked = [this] (te::MidiClip* c) {
            if (onRequestOpenPianoRoll)
                onRequestOpenPianoRoll (c);
        };

        // New: clipboard callbacks
        ui->onCopyRequested = [this] (te::MidiClip* c) {
            if (appEngine)
                appEngine->copyMidiClip (c);
        };

        ui->onDuplicateRequested = [this] (te::MidiClip* c) {
            if (appEngine)
            {
                appEngine->duplicateMidiClip (c);
                rebuildAndRefreshHighlight();
            }
        };

        ui->onPasteRequested = [this] (te::MidiClip* c, double pasteBeats) {
            juce::ignoreUnused (c); // track determination is based on this component's trackIndex
            if (appEngine)
            {
                appEngine->pasteClipboardAt (trackIndex, pasteBeats);
                rebuildAndRefreshHighlight();
            }
        };

        ui->onDeleteRequested = [this] (te::MidiClip* c) {
            if (auto* parent = findParentComponentOfClass<TrackEditView>())
            {
                // If the piano roll is currently showing a clip from this track,
                // hide it before removing the clip to avoid dangling UI state.
                if (parent->getPianoRollIndex() == trackIndex)
                    parent->hidePianoRoll();
            }

            if (appEngine)
            {
                appEngine->deleteMidiClip (c);
                rebuildClipsFromEngine();
                resized();
            }
        };

        // Right-click context menu is owned by TrackComponent now
        ui->onContextMenuRequested = [this] (te::MidiClip* c) {
            if (c == nullptr)
                return;

            juce::PopupMenu m;
            m.addItem (1, "Copy");
            m.addItem (2, "Duplicate");
            m.addSeparator();
            m.addItem (3, "Delete");

            m.showMenuAsync ({}, [safeThis = juce::Component::SafePointer<TrackComponent> (this), clip = c] (int result) {
                if (safeThis == nullptr || safeThis->appEngine == nullptr)
                    return;

                switch (result)
                {
                    case 1: // Copy
                        safeThis->appEngine->copyMidiClip (clip);
                        break;
                    case 2: // Duplicate
                        safeThis->appEngine->duplicateMidiClip (clip);
                        safeThis->rebuildAndRefreshHighlight();
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

        // Drag callbacks - Written by Claude Code
        ui->onDragUpdate = [this, clip = mc] (int targetTrack, t::TimePosition time, t::TimeDuration length, bool isValid) {
            auto* tl = findParentComponentOfClass<TrackListComponent>();
            if (!tl)
                return;

            // Validate the drop location
            const bool canMove = tl->canClipMoveToTrack (clip, trackIndex, targetTrack);
            const auto targetRange = t::TimeRange (time, time + length);
            const bool hasOverlap = tl->wouldClipOverlap (clip, targetTrack, targetRange);

            const bool validDrop = canMove && !hasOverlap;

            // Show ghost preview at quantized position
            tl->showGhostClip (targetTrack, time, length, validDrop);
        };

        ui->onDragComplete = [this] (te::MidiClip* clip, int targetTrack, t::TimePosition newStart) {
            auto* tl = findParentComponentOfClass<TrackListComponent>();
            if (!tl || !clip || !appEngine)
            {
                if (tl) tl->hideGhostClip();
                return;
            }

            // Hide ghost
            tl->hideGhostClip();

            // Validate final drop location
            const bool canMove = tl->canClipMoveToTrack (clip, trackIndex, targetTrack);
            const auto clipLength = clip->getPosition().getLength();
            const auto targetRange = t::TimeRange (newStart, newStart + clipLength);
            const bool hasOverlap = tl->wouldClipOverlap (clip, targetTrack, targetRange);

            if (!canMove || hasOverlap)
            {
                // Invalid drop - do nothing
                return;
            }

            // Apply changes to model
            const bool changingTracks = (targetTrack != trackIndex);

            // First, move clip to new time position
            clip->setStart (newStart, false, true); // preserveSync=false, keepLength=true

            // If changing tracks, move clip to target track
            if (changingTracks)
            {
                auto audioTracks = te::getAudioTracks (appEngine->getEdit());
                if (targetTrack >= 0 && targetTrack < audioTracks.size())
                {
                    auto* targetTrackPtr = audioTracks[targetTrack];
                    if (targetTrackPtr)
                    {
                        clip->moveTo (*targetTrackPtr);
                    }
                }
            }

            // Rebuild UI for affected tracks
            if (changingTracks)
            {
                // Rebuild both source and target tracks
                tl->rebuildTrack (trackIndex);     // Source track
                tl->rebuildTrack (targetTrack);     // Target track

                // Restore clip editing state if piano roll is open (Written by Claude Code)
                if (auto* parent = findParentComponentOfClass<TrackEditView>())
                    parent->refreshClipEditState();
            }
            else
            {
                // Just moving within same track
                rebuildAndRefreshHighlight();
            }
        };

        addAndMakeVisible (ui.get());
        clipUIs.add (std::move (ui));
    }

    // Extend TrackComponent length to fit all MIDI clips
    if (auto* list = findParentComponentOfClass<TrackListComponent>())
    {
        int rightmost = 0;
        // TrackComponent X is the left offset inside TrackList
        const int trackLeftInList = getX();

        for (auto* ui : clipUIs)
            if (ui)
                rightmost = std::max (rightmost, trackLeftInList + ui->getRight());

        // Account for the header column (same value used in TrackListComponent)
        constexpr int headerWidth = 140;
        const int requiredWidth = headerWidth + std::max (rightmost, getParentWidth() - headerWidth);

        if (requiredWidth > list->getWidth())
            list->setSize (requiredWidth + 40 /* small pad */, list->getHeight());
    }

    resized(); // redraw
}

void TrackComponent::updateClipEditedState (te::MidiClip* editedClip)
{
    // Update visual state for all clips to show which one is being edited (Written by Claude Code)
    for (auto* ui : clipUIs)
    {
        if (ui)
            ui->setBeingEdited (ui->getMidiClip() == editedClip);
    }
}

// Written by Claude Code
void TrackComponent::rebuildAndRefreshHighlight()
{
    rebuildClipsFromEngine();
    resized();

    // Trigger parent to recalculate size based on new clip positions (Written by Claude Code)
    if (auto* list = findParentComponentOfClass<TrackListComponent>())
        list->resized();

    // Restore clip highlight if piano roll is open
    if (auto* parent = findParentComponentOfClass<TrackEditView>())
        parent->refreshClipEditState();
}

void TrackComponent::mouseUp (const juce::MouseEvent& e)
{
    // Only handle background right-clicks on the TrackComponent itself
    // If the original component was a child (e.g., a TrackClip), let it handle its own menu
    if (!e.mods.isPopupMenu() || e.originalComponent != this)
        return;

    // Compute start position from X using beat-based coordinates
    const auto inner = getLocalBounds().reduced (5);
    const int localX = juce::jmax (0, e.getPosition().x - inner.getX());

    auto* tl = findParentComponentOfClass<TrackListComponent>();
    const double pixelsPerBeat = tl ? tl->getPixelsPerBeat() : 100.0;
    const double viewStartBeats = tl ? tl->getViewStartBeat().inBeats() : 0.0;

    const double clickBeats = viewStartBeats + (localX / juce::jmax (1.0, pixelsPerBeat));

    // Quantize to 0.25 beat grid (Written by Claude Code)
    constexpr double gridSize = 0.25;
    const double quantizedBeats = std::round (clickBeats / gridSize) * gridSize;

    // Convert quantized beats to TimePosition using tempo sequence (Written by Claude Code)
    const t::TimePosition startPos = appEngine->getEdit().tempoSequence.toTime(t::BeatPosition::fromBeats(quantizedBeats));

    juce::PopupMenu m;

    // Only show paste option if clipboard has compatible content (Junie)
    if (appEngine && appEngine->canPasteToTrack (trackIndex))
        m.addItem (1, "Paste Here");

    m.addItem (2, "Add MIDI Clip Here");

    m.showMenuAsync ({}, [safeThis = juce::Component::SafePointer<TrackComponent> (this), startPos] (int result)
    {
        if (safeThis == nullptr || safeThis->appEngine == nullptr)
            return;

        switch (result)
        {
            case 1: // Paste Here
            {
                // Get clipboard clip length and check for overlap (Written by Claude Code)
                const double clipLengthBeats = safeThis->appEngine->getClipboardClipLengthBeats();
                const double pasteBeats = safeThis->appEngine->getEdit().tempoSequence.toBeats (startPos).inBeats();
                const double endBeats = pasteBeats + clipLengthBeats;

                // Check if paste would overlap with existing clips
                const auto pasteStartTime = safeThis->appEngine->getEdit().tempoSequence.toTime (t::BeatPosition::fromBeats (pasteBeats));
                const auto pasteEndTime = safeThis->appEngine->getEdit().tempoSequence.toTime (t::BeatPosition::fromBeats (endBeats));
                const t::TimeRange pasteRange (pasteStartTime, pasteEndTime);

                // Check all clips on track for overlap
                bool wouldOverlap = false;
                auto clipsOnTrack = safeThis->appEngine->getMidiClipsFromTrack (safeThis->trackIndex);
                for (auto* existingClip : clipsOnTrack)
                {
                    auto existingRange = existingClip->getPosition().time;
                    if (pasteRange.overlaps (existingRange))
                    {
                        wouldOverlap = true;
                        break;
                    }
                }

                // Only paste if no overlap
                if (!wouldOverlap)
                {
                    if (safeThis->appEngine->pasteClipboardAt (safeThis->trackIndex, pasteBeats))
                        safeThis->rebuildAndRefreshHighlight();
                }
                break;
            }
            case 2: // Add MIDI Clip Here
            {
                // Check for overlap before creating clip (Written by Claude Code)
                const double clipLengthBeats = 4.0;
                const auto startBeats = safeThis->appEngine->getEdit().tempoSequence.toBeats (startPos).inBeats();
                const auto endBeats = startBeats + clipLengthBeats;

                const auto clipStartTime = safeThis->appEngine->getEdit().tempoSequence.toTime (t::BeatPosition::fromBeats (startBeats));
                const auto clipEndTime = safeThis->appEngine->getEdit().tempoSequence.toTime (t::BeatPosition::fromBeats (endBeats));
                const t::TimeRange clipRange (clipStartTime, clipEndTime);

                // Check all clips on track for overlap
                bool wouldOverlap = false;
                auto clipsOnTrack = safeThis->appEngine->getMidiClipsFromTrack (safeThis->trackIndex);
                for (auto* existingClip : clipsOnTrack)
                {
                    auto existingRange = existingClip->getPosition().time;
                    if (clipRange.overlaps (existingRange))
                    {
                        wouldOverlap = true;
                        break;
                    }
                }

                // Only create if no overlap
                if (!wouldOverlap)
                {
                    const auto length = t::BeatDuration::fromBeats (clipLengthBeats);
                    if (safeThis->appEngine->addMidiClipToTrackAt (safeThis->trackIndex, startPos, length))
                        safeThis->rebuildAndRefreshHighlight();
                }
                break;
            }
            default:
                break;
        }
    });
}
