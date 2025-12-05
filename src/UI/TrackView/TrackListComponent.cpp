#include "TrackListComponent.h"
#include "DrumSamplerView/DrumSamplerView.h"
#include "TrackEditView.h"
#include <tracktion_engine/tracktion_engine.h>
namespace t = tracktion;
namespace te = tracktion::engine;
#include "TrackListComponent.h"

TrackListComponent::TrackListComponent (const std::shared_ptr<AppEngine>& engine)
    : appEngine (engine),
      playhead (engine->getEdit(),
          engine->getEditViewState(),
          *engine),
      loopRangeComponent (engine->getEdit())
{
    //Add initial track pair
    //addNewTrack();
    setWantsKeyboardFocus (true); // setting keyboard focus?
    addAndMakeVisible (playhead);
    playhead.setAlwaysOnTop (true);

    addAndMakeVisible (loopRangeComponent);
    loopRangeComponent.setAlwaysOnTop (true);

    // Set up playhead with beat-based coordinates
    playhead.setPixelsPerBeat(100.0);
    playhead.setViewStartBeat(t::BeatPosition::fromBeats(0.0));

    // Set up loop range component with beat-based coordinates
    loopRangeComponent.setPixelsPerBeat(100.0);
    loopRangeComponent.setViewStartBeat(t::BeatPosition::fromBeats(0.0));

    // Set up timeline with beat-based coordinates
    timeline = std::make_unique<ui::TimelineComponent>(appEngine->getEdit());
    addAndMakeVisible (timeline.get());
    timeline->setPixelsPerBeat (100.0);
    timeline->setViewStartBeat (t::BeatPosition::fromBeats (0.0));
    timeline->setEditForSnap(&appEngine->getEdit());
    timeline->setSnapToBeats(true);

    addAndMakeVisible(loopButton);
    loopButton.setClickingTogglesState(true);

    // use the same colour as the loop region
    const auto loopColour = juce::Colours::darkorange;

    // initial UI state based on transport
    {
        auto& tr = appEngine->getEdit().getTransport();
        loopButton.setToggleState(tr.looping, juce::dontSendNotification);
        loopButton.setColour(juce::TextButton::buttonColourId, tr.looping ? loopColour : juce::Colours::darkgrey);
        loopRangeComponent.setLooping(tr.looping);
        loopRangeComponent.setLoopRange(tr.getLoopRange());
    }
    // toggle handler
    loopButton.onClick = [this, loopColour]
    {
        auto& tr = appEngine->getEdit().getTransport();
        const bool enable = loopButton.getToggleState();

        if (enable)
        {
            auto r = timeline->getLoopRange();
            if (r.getLength().inSeconds() <= 0.0)
            {
                // seed ONCE (4 beats = 1 bar)
                const double startBeats = timeline->getViewStartBeat().inBeats();
                const double start = appEngine->getEdit().tempoSequence.toTime(t::BeatPosition::fromBeats(startBeats)).inSeconds();
                r = t::TimeRange(t::TimePosition::fromSeconds(start),
                                         t::TimePosition::fromSeconds(start + 4.0));
                timeline->setLoopRange(r);
                tr.setLoopRange(r);
                tr.setPosition(r.getStart());
            }
            tr.looping = true;
            // (optional) tr.ensureContextAllocated();
            loopButton.setColour(juce::TextButton::buttonColourId, loopColour);
            loopRangeComponent.setLooping(true);
            loopRangeComponent.setLoopRange(r);
        }
        else
        {
            tr.looping = false;
            loopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            loopRangeComponent.setLooping(false);
        }
    };

    timeline->onLoopRangeChanged = [this](t::TimeRange r)
    {
        auto& tr = appEngine->getEdit().getTransport();

        const bool hasLoop = r.getLength().inSeconds() > 0.0;
        if (hasLoop)
        {
            tr.setLoopRange(r);
            tr.looping = true;

            const auto pos = tr.getPosition();
            if (pos < r.getStart() || pos >= r.getEnd())
                tr.setPosition(r.getStart());
        }
        else
        {
            tr.looping = false;
        }

        // reflect in the button and loop range component
        const auto loopColour = juce::Colours::darkorange;
        loopButton.setToggleState(tr.looping, juce::dontSendNotification);
        loopButton.setColour(juce::TextButton::buttonColourId, tr.looping ? loopColour : juce::Colours::darkgrey);
        loopRangeComponent.setLooping(tr.looping);
        loopRangeComponent.setLoopRange(r);
        repaint();
    };

    appEngine->onArmedTrackChanged = [this] {
        refreshTrackStates();
    };

    appEngine->onInstrumentLabelChanged = [this] (int trackIdx)
    {
        if (trackIdx < 0 || trackIdx >= headers.size())
            return;

        if (auto* header = headers[trackIdx])
        {
            header->setInstrumentLabel (
                appEngine->getInstrumentLabelForTrack (trackIdx));
        }
    };

    // Handle recording stopped: refresh clips on armed track
    appEngine->onRecordingStopped = [this] {
        const int armedIndex = appEngine->getArmedTrackIndex();
        juce::Logger::writeToLog("[TrackListComponent] Recording stopped, armed track index: " + juce::String(armedIndex));

        if (armedIndex >= 0 && armedIndex < tracks.size())
        {
            juce::Logger::writeToLog("[TrackListComponent] Rebuilding clips for track " + juce::String(armedIndex));
            tracks[armedIndex]->rebuildClipsFromEngine();
            tracks[armedIndex]->resized();
            repaint();
        }
        else
        {
            juce::Logger::writeToLog("[TrackListComponent] Armed track index out of range: " + juce::String(armedIndex) + " (num tracks: " + juce::String(tracks.size()) + ")");
        }
    };

    // Handle BPM changes: maintain beat positions for loop range and playhead
    appEngine->onBpmChanged = [this](double oldBpm, double newBpm, t::TimeRange oldLoopRange, t::TimePosition oldPlayheadPos)
    {
        auto& tr = appEngine->getEdit().getTransport();
        auto& tempoSeq = appEngine->getEdit().tempoSequence;

        // Update loop range to maintain beat positions
        // Use the ORIGINAL loop range values (before Tracktion adjusted them)
        if (oldLoopRange.getLength().inSeconds() > 0.0)
        {
            // Convert original time to beats using old BPM (manual calculation)
            // At a given BPM: beats = seconds * (BPM / 60)
            const double startBeats = oldLoopRange.getStart().inSeconds() * (oldBpm / 60.0);
            const double endBeats = oldLoopRange.getEnd().inSeconds() * (oldBpm / 60.0);

            // Convert beats back to time using new tempo sequence
            const auto startTime = tempoSeq.toTime(t::BeatPosition::fromBeats(startBeats));
            const auto endTime = tempoSeq.toTime(t::BeatPosition::fromBeats(endBeats));
            const auto newLoopRange = t::TimeRange(startTime, endTime);

            // Update both transport and timeline
            tr.setLoopRange(newLoopRange);
            if (timeline)
                timeline->setLoopRange(newLoopRange);
            loopRangeComponent.setLoopRange(newLoopRange);
        }

        // Update playhead position to maintain beat position (only if not playing)
        // Use the ORIGINAL playhead position (before Tracktion adjusted it)
        if (!tr.isPlaying())
        {
            // Convert original position to beats using old BPM
            const double posBeats = oldPlayheadPos.inSeconds() * (oldBpm / 60.0);

            // Convert back to time using new tempo
            const auto newPos = tempoSeq.toTime(t::BeatPosition::fromBeats(posBeats));

            tr.setPosition(newPos);
        }

        repaint();
    };
}

/**
 * @brief Destructor. Clears AppEngine callbacks to prevent use-after-free crashes.
 *
 * **CRITICAL**: UI components that register callbacks on AppEngine MUST clear them in
 * destructors. AppEngine typically outlives UI components (e.g., when switching views).
 * If callbacks capture `this` pointer, invoking them after component destruction causes
 * segmentation fault.
 *
 * **Bug History**: BPM change crashed with segfault when external plugins loaded (Nov 19, 2025).
 * Root cause: onBpmChanged callback captured `this` pointer that became dangling when
 * TrackListComponent was destroyed but AppEngine remained alive.
 *
 * **Pattern**: All UI components registering AppEngine callbacks should follow this pattern.
 *
 * @see AppEngine::onBpmChanged
 * @see AppEngine::onArmedTrackChanged
 */
TrackListComponent::~TrackListComponent()
{
    // Clear callbacks to prevent use-after-free when AppEngine outlives this component
    if (appEngine)
    {
        appEngine->onBpmChanged = nullptr;
        appEngine->onArmedTrackChanged = nullptr;
    }
}

void TrackListComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF343A40)); // Dark background for track area
    // Loop range lines are now drawn by LoopRangeComponent
}

void TrackListComponent::resized()
{
    constexpr int headerWidth    = 140;
    constexpr int trackHeight    = 125;
    constexpr int addButtonSpace = 30;

    const int numTracks = juce::jmin(headers.size(), tracks.size());
    const int contentH  = numTracks * trackHeight + addButtonSpace;

    // 1) First pass: lay out timeline, headers, and tracks
    auto bounds = getLocalBounds();

    // timeline
    {
        auto timelineRow = bounds.removeFromTop(timelineHeight);
        auto controls = timelineRow.removeFromLeft(headerWidth);

        if (loopButton.isShowing() || loopButton.getParentComponent() == this)
            loopButton.setBounds(controls.reduced(6, 4));

        if (timeline)
            timeline->setBounds(timelineRow);
    }

    bounds.removeFromBottom(addButtonSpace);

    // rows
    for (int i = 0; i < numTracks; ++i)
    {
        constexpr int margin = 2;
        auto row = bounds.removeFromTop(trackHeight);

        if (headers[i] != nullptr)
            headers[i]->setBounds(row.removeFromLeft(headerWidth).reduced(margin));

        if (tracks[i] != nullptr)
            tracks[i]->setBounds(row.reduced(margin));  // <-- this triggers TrackComponent::resized()
    }

    // playhead and loop range over everything
    const auto overlayBounds = getLocalBounds()
                                   // .withTrimmedBottom(addButtonSpace)
                                   .withTrimmedLeft(headerWidth)
                                   .withTrimmedTop (timelineHeight);

    playhead.setBounds(overlayBounds);
    playhead.toFront(false);

    loopRangeComponent.setBounds(overlayBounds);
    loopRangeComponent.toFront(false);

    // 2) Second pass: now that clip UIs have bounds, compute true rightmost
    int rightmostClipPx = 0;
    for (auto* t : tracks)
    {
        if (!t) continue;
        const int trackLeftInList = t->getX();
        for (int i = 0; i < t->getNumChildComponents(); ++i)
        {
            if (auto* child = t->getChildComponent(i))
                rightmostClipPx = std::max(rightmostClipPx, trackLeftInList + child->getRight());
        }
    }

    // Calculate width based on beat length instead of time length
    const auto editLengthTime = appEngine->getEdit().getLength();
    const auto editEndPos = t::TimePosition::fromSeconds(editLengthTime.inSeconds());
    const double beats = appEngine->getEdit().tempoSequence.toBeats(editEndPos).inBeats();
    const double ppb = timeline ? timeline->getPixelsPerBeat() : 100.0;
    const int widthByEdit = (int) juce::roundToInt(beats * ppb);
    const int    parentW     = getParentComponent() ? getParentComponent()->getWidth() : getWidth();

    const int bodyMinW = std::max({ widthByEdit, rightmostClipPx, parentW - headerWidth });
    const int desiredW = headerWidth + std::max(bodyMinW, 800);

    // Calculate total height needed: timeline + tracks + button space
    // Use the GREATER of contentH+timeline or parent height to:
    // - Fill viewport when few tracks exist (prevents grey gap, playhead fills space)
    // - Extend beyond viewport when many tracks exist (prevents squishing)
    const int totalContentH = timelineHeight + contentH;
    const int parentH = getParentComponent() ? getParentComponent()->getHeight() : getHeight();
    const int desiredH = std::max(totalContentH, parentH);

    if (desiredW != getWidth() || desiredH != getHeight())
    {
        setSize(desiredW, desiredH);
    }
}

void TrackListComponent::addNewTrack (int engineIdx)
{
    // Select random color from palette
    const auto newColor = trackColors[tracks.size() % trackColors.size()];

    auto* header = new TrackHeaderComponent(*appEngine); // Written by Claude Code - pass AppEngine reference
    header->setTrackIndex (engineIdx); // Set track index for renaming (Written by Claude Code)
    auto* newTrack = new TrackComponent (appEngine, engineIdx, newColor);
    // newTrack->setEngineIndex (engineIdx);

    header->setTrackIndex (newTrack->getTrackIndex());
    header->setInstrumentLabel (appEngine->getInstrumentLabelForTrack (newTrack->getTrackIndex()));

    newTrack->setPixelsPerBeat (100.0);
    newTrack->setViewStartBeat (t::BeatPosition::fromBeats(0.0));

    header->addListener (newTrack);

    // Load track name from engine (Written by Claude Code)
    const bool isDrum = appEngine->isDrumTrack (newTrack->getTrackIndex());
    juce::String trackName = appEngine->getTrackName (engineIdx);
    // Fallback to default names if engine doesn't have a name set (shouldn't happen now)
    if (trackName.isEmpty())
    {
        // Use overall track position for numbering (1-indexed)
        trackName = isDrum ? ("Drums " + juce::String (engineIdx + 1))
                           : ("Track " + juce::String (engineIdx + 1));
    }
    header->setTrackName (trackName);

    headers.add (header);

    header->setTrackType (isDrum ? TrackHeaderComponent::TrackType::Drum
                                 : TrackHeaderComponent::TrackType::Instrument);

    tracks.add (newTrack);

    addAndMakeVisible (header);
    addAndMakeVisible (newTrack);

    updateTrackIndexes();

    header->setMuted (appEngine->isTrackMuted (newTrack->getTrackIndex()));
    header->setSolo (appEngine->isTrackSoloed (newTrack->getTrackIndex()));
    refreshTrackStates();

    newTrack->onRequestDeleteTrack = [this] (int uiIndex) {
        if (uiIndex >= 0 && uiIndex < tracks.size() && uiIndex < headers.size())
        {
            if (auto* parent = findParentComponentOfClass<TrackEditView>())
                if (parent->getPianoRollIndex() == uiIndex)
                    parent->hidePianoRoll();

            if (headers[uiIndex] != nullptr)
                removeChildComponent (headers[uiIndex]);
            if (tracks[uiIndex] != nullptr)
                removeChildComponent (tracks[uiIndex]);
            headers.remove (uiIndex);
            tracks.remove (uiIndex);
            appEngine->deleteMidiTrack (uiIndex);
            updateTrackIndexes();
            refreshTrackStates(); // Refresh solo/mute/arm states after track deletion
            resized();
        }

    };

    newTrack->onRequestOpenPianoRoll = [this] (te::MidiClip* clip) {
        if (auto* parent = findParentComponentOfClass<TrackEditView>())
            parent->showPianoRoll (clip);
    };

    newTrack->onRequestOpenDrumSampler = [this] (int uiIndex) {
        if (uiIndex < 0 || uiIndex >= tracks.size())
            return;

        if (auto* eng = appEngine->getDrumAdapter (uiIndex))
        {
            auto* comp = new DrumSamplerView (static_cast<DrumSamplerEngine&> (*eng));

            juce::DialogWindow::LaunchOptions opts;
            comp->setSize (1000, 700);
            opts.content.setOwned (comp);
            opts.dialogTitle = "Drum Sampler";
            opts.resizable = true;
            opts.useNativeTitleBar = true;

            opts.launchAsync();
        }
    };

    resized();
}

void TrackListComponent::parentSizeChanged()
{
    resized();
}

void TrackListComponent::updateTrackIndexes() const
{
    const int n = tracks.size();
    for (int i = 0; i < n; ++i)
    {
        if (tracks[i] != nullptr)
            tracks[i]->setTrackIndex (i);
        // Update header trackIndex too (Written by Claude Code)
        if (i < headers.size() && headers[i] != nullptr)
            headers[i]->setTrackIndex (i);
    }
}

void TrackListComponent::refreshTrackStates() const
{
    const bool anySolo = appEngine->anyTrackSoloed();
    const int n = juce::jmin (headers.size(), tracks.size());
    for (int i = 0; i < n; ++i)
    {
        const bool thisSolo = appEngine->isTrackSoloed (tracks[i] != nullptr ? tracks[i]->getTrackIndex() : i);
        if (headers[i] != nullptr)
            headers[i]->setDimmed (anySolo && !thisSolo);
    }

    for (int i = 0; i < headers.size(); ++i)
    {
        if (headers[i] != nullptr)
            headers[i]->setArmed (appEngine->getArmedTrackIndex() == i);
    }
}

void TrackListComponent::armTrack (int trackIndex, bool shouldBeArmed)
{
    const int newIndex = shouldBeArmed ? trackIndex : -1;
    if (appEngine->getArmedTrackIndex() != newIndex)
        appEngine->setArmedTrack (newIndex);
}

void TrackListComponent::setAllArmButtonsEnabled(bool enabled)
{
    // Loop through all headers and set enabled or disabled state
    for (TrackHeaderComponent *header : headers)
    {
        if (header != nullptr)
        {
            header->setArmButtonEnabled (enabled);
        }
    }
}

void TrackListComponent::repaintTrack(int trackIndex)
{
    if (trackIndex >= 0 && trackIndex < tracks.size())
    {
        if (auto* track = tracks[trackIndex])
            track->repaint();
    }
}

void TrackListComponent::setPixelsPerBeat (double ppb)
{
    // Update all components to use beat-based coordinates
    for (auto* t : tracks) if (t) t->setPixelsPerBeat (ppb);
    if (timeline) timeline->setPixelsPerBeat (ppb);
    playhead.setPixelsPerBeat(ppb);
    loopRangeComponent.setPixelsPerBeat(ppb);
    repaint();
}

void TrackListComponent::setViewStartBeat (t::BeatPosition b)
{
    // Update all components to use beat-based coordinates
    for (auto* tc : tracks) if (tc) tc->setViewStartBeat (b);
    if (timeline) timeline->setViewStartBeat (b);
    playhead.setViewStartBeat(b);
    loopRangeComponent.setViewStartBeat(b);
    repaint();
}

void TrackListComponent::rebuildFromEngine()
{
    for (auto* h : headers) removeChildComponent(h);
    for (auto* t : tracks)  removeChildComponent(t);
    headers.clear(); tracks.clear();

    const int n = appEngine->getNumTracks();
    for (int i = 0; i < n; ++i)
        addNewTrack(i);

    resized();
}

// Written by Claude Code
void TrackListComponent::rebuildTrack (int trackIndex)
{
    if (trackIndex >= 0 && trackIndex < tracks.size())
    {
        if (auto* track = tracks[trackIndex])
        {
            track->rebuildClipsFromEngine();
            track->resized();
        }
    }
}

// Written by Claude Code
void TrackListComponent::updateClipEditState (int trackIndex, te::MidiClip* editedClip)
{
    if (trackIndex >= 0 && trackIndex < tracks.size())
    {
        if (auto* track = tracks[trackIndex])
            track->updateClipEditedState (editedClip);
    }
}

// Drag validation helpers - Written by Claude Code
bool TrackListComponent::canClipMoveToTrack (te::MidiClip* clip, int sourceTrack, int targetTrack) const
{
    if (!appEngine || !clip)
        return false;

    // Check if target track index is valid
    if (targetTrack < 0 || targetTrack >= appEngine->getNumTracks())
        return false;

    // Allow movement within same track
    if (sourceTrack == targetTrack)
        return true;

    // Check track type matching (drum to drum, instrument to instrument)
    const bool sourceIsDrum = appEngine->isDrumTrack (sourceTrack);
    const bool targetIsDrum = appEngine->isDrumTrack (targetTrack);

    return sourceIsDrum == targetIsDrum;
}

bool TrackListComponent::wouldClipOverlap (te::MidiClip* clipToMove, int targetTrack, t::TimeRange range) const
{
    if (!appEngine || !clipToMove)
        return false;

    // Get all MIDI clips from target track
    auto clips = appEngine->getMidiClipsFromTrack (targetTrack);

    for (auto* existingClip : clips)
    {
        // Skip the clip being moved
        if (existingClip == clipToMove)
            continue;

        // Check if time ranges overlap
        auto existingRange = existingClip->getPosition().time;
        if (range.overlaps (existingRange))
            return true;
    }

    return false;
}

int TrackListComponent::getTrackIndexAtY (int y) const
{
    constexpr int trackHeight = 125;

    // Account for timeline at top
    const int yRelative = y - timelineHeight;
    if (yRelative < 0)
        return -1;

    const int index = yRelative / trackHeight;

    // Validate index
    if (index < 0 || index >= appEngine->getNumTracks())
        return -1;

    return index;
}

// Ghost clip management - Written by Claude Code (fixed seconds/beats conversion)
void TrackListComponent::showGhostClip (int trackIndex, t::TimePosition time,
                                        t::TimeDuration length, bool isValid)
{
    // Create ghost if it doesn't exist
    if (!ghostClip)
    {
        ghostClip = std::make_unique<GhostClipComponent>();
        addAndMakeVisible (ghostClip.get());
    }

    // Calculate ghost bounds using same logic as TrackComponent::resized()
    constexpr int trackHeight = 125;
    const double pixelsPerBeat = getPixelsPerBeat();
    const double viewStartBeats = getViewStartBeat().inBeats();

    // Convert time positions to beat positions for layout
    auto& tempoSeq = appEngine->getEdit().tempoSequence;
    const double clipStartBeats = tempoSeq.toBeats (time).inBeats();
    const auto clipEndTime = time + length;
    const double clipEndBeats = tempoSeq.toBeats (clipEndTime).inBeats();
    const double clipLenBeats = clipEndBeats - clipStartBeats;

    const int timelineX = static_cast<int> (juce::roundToIntAccurate ((clipStartBeats - viewStartBeats) * pixelsPerBeat));
    const int w = static_cast<int> (juce::roundToIntAccurate (clipLenBeats * pixelsPerBeat));
    const int y = timelineHeight + (trackIndex * trackHeight) + 5; // +5 for inner margin
    const int h = trackHeight - 10; // Account for margins

    // Ghost is positioned in TrackListComponent coordinates, so add headerWidth offset (Written by Claude Code)
    const int x = headerWidth + timelineX;

    ghostClip->setBounds (x, y, juce::jmax (w, 20), h);
    ghostClip->setDropLocation (trackIndex, time, length, isValid);
    ghostClip->show();
}

void TrackListComponent::hideGhostClip()
{
    if (ghostClip)
        ghostClip->hide();
}

// bool EditComponent::keyPressed(const KeyPress& key) {
//     if (key == KeyPress::deleteKey) {
//         removeSelectedTracks();
//         return true;
//     }
//     return false;
// }

// void EditComponent::removeSelectedTracks() {
//     // Remove in reverse order to avoid index issues
//     for (int i = headers.size() - 1; i >= 0; --i) {
//         if (headers[i]->isSelected()) {
//             removeChildComponent(headers[i]);
//             removeChildComponent(tracks[i]);
//
//             headers.remove(i);
//             tracks.remove(i);
//         }
//     }
//     resized();
// }

bool TrackListComponent::keyStateChanged (bool isKeyDown)
{
    if (auto* tev = findParentComponentOfClass<TrackEditView>())
        return tev->keyStateChanged (isKeyDown);
    return false;
}

bool TrackListComponent::keyPressed (const juce::KeyPress& kp)
{
    if (auto* tev = findParentComponentOfClass<TrackEditView>())
        return tev->keyPressed (kp);
    return false;
}

void TrackListComponent::parentHierarchyChanged()
{
    juce::MessageManager::callAsync(
        [safe = juce::Component::SafePointer<TrackListComponent>(this)]
        {
            if (safe != nullptr && safe->isShowing())
                safe->grabKeyboardFocus();
        });
}

void TrackListComponent::mouseDown (const juce::MouseEvent& e)
{
    grabKeyboardFocus();                 // user click = fine
    juce::Component::mouseDown(e);
}

