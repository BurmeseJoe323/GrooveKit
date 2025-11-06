#include "DrumSamplerView/DrumSamplerView.h"
#include "TrackEditView.h"
#include "TrackListComponent.h"

TrackListComponent::TrackListComponent (const std::shared_ptr<AppEngine>& engine) : appEngine (engine),
                                                                                    playhead (engine->getEdit(),
                                                                                        engine->getEditViewState())
{
    //Add initial track pair
    //addNewTrack();
    setWantsKeyboardFocus (true); // setting keyboard focus?
    addAndMakeVisible (playhead);
    playhead.setAlwaysOnTop (true);

    // Set up playhead
    playhead.setPixelsPerSecond(100.0);
    playhead.setViewStart(te::TimePosition::fromSeconds(0.0));

    // Set up timeline
    timeline = std::make_unique<ui::TimelineComponent>(appEngine->getEdit());
    addAndMakeVisible (timeline.get());
    timeline->setPixelsPerSecond (100.0);
    timeline->setViewStart (te::TimePosition::fromSeconds (0.0));
    timeline->setEditForSnap(&appEngine->getEdit());
    timeline->setSnapToBeats(true);

    // Add callback for timeline component that's listening to playhead
    playhead.onPlayheadMoved = [this] () { timeline->repaint(); };

    addAndMakeVisible(loopButton);
    loopButton.setClickingTogglesState(true);

    // use the same colour as the loop region
    const auto loopColour = juce::Colours::darkorange;

    // initial UI state based on transport
    {
        auto& tr = appEngine->getEdit().getTransport();
        loopButton.setToggleState(tr.looping, juce::dontSendNotification);
        loopButton.setColour(juce::TextButton::buttonColourId, tr.looping ? loopColour : juce::Colours::darkgrey);
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
                // seed ONCE (4s example)
                const double start = timeline->getViewStart().inSeconds();
                r = tracktion::TimeRange(tracktion::TimePosition::fromSeconds(start),
                                         tracktion::TimePosition::fromSeconds(start + 4.0));
                timeline->setLoopRange(r);
                tr.setLoopRange(r);
                tr.setPosition(r.getStart());
            }
            tr.looping = true;
            // (optional) tr.ensureContextAllocated();
            loopButton.setColour(juce::TextButton::buttonColourId, loopColour);
        }
        else
        {
            tr.looping = false;
            loopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        }
    };

    timeline->onLoopRangeChanged = [this](tracktion::TimeRange r)
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

        // reflect in the button
        const auto loopColour = juce::Colours::darkorange;
        loopButton.setToggleState(tr.looping, juce::dontSendNotification);
        loopButton.setColour(juce::TextButton::buttonColourId, tr.looping ? loopColour : juce::Colours::darkgrey);
    };

    appEngine->onArmedTrackChanged = [this] {
        refreshTrackStates();
    };
}

TrackListComponent::~TrackListComponent() = default;

void TrackListComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF343A40)); // Dark background for track area
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

    // playhead over everything
    playhead.setBounds(getLocalBounds()
                           .withTrimmedBottom(addButtonSpace)
                           .withTrimmedLeft(headerWidth)
                           .withTrimmedTop (timelineHeight));
    playhead.toFront(false);

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

    const double seconds    = appEngine->getEdit().getLength().inSeconds();
    const double pps        = timeline ? timeline->getPixelsPerSecond() : 100.0;
    const int    widthByEdit = (int) juce::roundToInt(seconds * pps);
    const int    parentW     = getParentComponent() ? getParentComponent()->getWidth() : getWidth();

    const int bodyMinW = std::max({ widthByEdit, rightmostClipPx, parentW - headerWidth });
    const int desiredW = headerWidth + std::max(bodyMinW, 800);
    const int desiredH = std::max(contentH, getParentHeight());

    if (desiredW != getWidth() || desiredH != getHeight())
    {
        setSize(desiredW, desiredH);
    }
}

void TrackListComponent::addNewTrack (int engineIdx)
{
    // Select random color from palette
    const auto newColor = trackColors[tracks.size() % trackColors.size()];

    auto* header = new TrackHeaderComponent();
    auto* newTrack = new TrackComponent (appEngine, engineIdx, newColor);
    // newTrack->setEngineIndex (engineIdx);

    newTrack->setPixelsPerSecond (100.0);
    newTrack->setViewStart (0s);

    header->addListener (newTrack);

    // Set the track name on the header
    const bool isDrum = appEngine->isDrumTrack (newTrack->getTrackIndex());
    header->setTrackName (isDrum ? "Drums" : ("Track " + juce::String (tracks.size() + 1)));

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
        if (tracks[i] != nullptr)
            tracks[i]->setTrackIndex (i);
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

void TrackListComponent::setPixelsPerSecond (double pps)
{
    for (auto* t : tracks) if (t) t->setPixelsPerSecond (pps);
    if (timeline) timeline->setPixelsPerSecond (pps);
    playhead.setPixelsPerSecond(pps);            // ← add this
    repaint();
}

void TrackListComponent::setViewStart (te::TimePosition t)
{
    for (auto* tc : tracks) if (tc) tc->setViewStart (t);
    if (timeline) timeline->setViewStart (t);
    playhead.setViewStart(t);                    // ← add this
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
