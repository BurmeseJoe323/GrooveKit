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
    selectedTrackIndex = 0;
}

TrackListComponent::~TrackListComponent() = default;

void TrackListComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF343A40)); // Dark background for track area
}

void TrackListComponent::resized()
{
    juce::FlexBox mainFlex;
    mainFlex.flexDirection = juce::FlexBox::Direction::column;

    constexpr int headerWidth = 140;
    constexpr int trackHeight = 125;
    constexpr int addButtonSpace = 30;

    const int numTracks = juce::jmin (headers.size(), tracks.size());
    const int contentH = numTracks * trackHeight + addButtonSpace;

    // Set the size to either default to the parent's height if the content height isn't tall enough
    setSize (getParentWidth(), contentH > getParentHeight() ? contentH : getParentHeight());

    auto bounds = getLocalBounds();
    bounds.removeFromBottom (addButtonSpace); // Space for add button
    for (int i = 0; i < numTracks; i++)
    {
        constexpr int margin = 2;
        // Header on left, track on right in same row
        auto row = bounds.removeFromTop (trackHeight);

        if (headers[i] != nullptr)
            headers[i]->setBounds (row.removeFromLeft (headerWidth).reduced (margin));
        if (tracks[i] != nullptr)
            tracks[i]->setBounds (row.reduced (margin));
    }

    // Set bounds for playhead
    playhead.setBounds (getLocalBounds().withTrimmedLeft (headerWidth));
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

    // header->setTrackType (
    //     appEngine->isDrumTrack (newTrack->getEngineIndex())
    //         ? TrackHeaderComponent::TrackType::Drum
    //         : TrackHeaderComponent::TrackType::Instrument);

    header->setTrackType (isDrum ? TrackHeaderComponent::TrackType::Drum
                                 : TrackHeaderComponent::TrackType::Instrument);

    tracks.add (newTrack);

    addAndMakeVisible (header);
    addAndMakeVisible (newTrack);

    updateTrackIndexes();

    header->setMuted (appEngine->isTrackMuted (newTrack->getTrackIndex()));
    header->setSolo (appEngine->isTrackSoloed (newTrack->getTrackIndex()));
    refreshSoloVisuals();

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

    newTrack->onRequestOpenPianoRoll = [this] (int uiIndex) {
        if (auto* parent = findParentComponentOfClass<TrackEditView>())
            parent->showPianoRoll (uiIndex);
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

void TrackListComponent::refreshSoloVisuals() const
{
    const bool anySolo = appEngine->anyTrackSoloed();
    const int n = juce::jmin (headers.size(), tracks.size());
    for (int i = 0; i < n; ++i)
    {
        const bool thisSolo = appEngine->isTrackSoloed (tracks[i] != nullptr ? tracks[i]->getTrackIndex() : i);
        if (headers[i] != nullptr)
            headers[i]->setDimmed (anySolo && !thisSolo);
    }
}

void TrackListComponent::setPixelsPerSecond (double pps)
{
    for (auto* t : tracks)
        if (t)
            t->setPixelsPerSecond (pps);
    repaint();
}

void TrackListComponent::setViewStart (te::TimePosition t)
{
    for (auto* tc : tracks)
        if (tc)
            tc->setViewStart (t);
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
