#include "TrackListComponent.h"
#include "DrumSamplerView/DrumSamplerLauncher.h"
#include "DrumSamplerView/DrumSamplerView.h"

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
    g.fillAll (juce::Colours::black);
}

void TrackListComponent::resized()
{
    juce::FlexBox mainFlex;
    mainFlex.flexDirection = juce::FlexBox::Direction::column;

    constexpr int headerWidth = 140;
    constexpr int trackHeight = 100;
    constexpr int addButtonSpace = 30;

    const int numTracks = headers.size();
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

        headers[i]->setBounds (row.removeFromLeft (headerWidth).reduced (margin));
        tracks[i]->setBounds (row.reduced (margin));
    }

    // Set bounds for playhead
    playhead.setBounds (getLocalBounds().withTrimmedLeft (headerWidth));
}

void TrackListComponent::addNewTrack (int engineIdx)
{
    auto* tPtr = appEngine->getTrackManager().getTrack (engineIdx);
    if (!tPtr) return;

    const auto newColor = trackColors[tracks.size() % trackColors.size()];

    auto* header   = new TrackHeaderComponent();
    auto* newTrack = new TrackComponent (appEngine.get(), tPtr, newColor);


    newTrack->setPixelsPerSecond (100.0);
    newTrack->setViewStart (0s);

    header->addListener (newTrack);
    header->setTrackName ("MIDI Track " + juce::String (engineIdx + 1));

    const int insertPos = juce::jlimit (0, tracks.size(), engineIdx);
    headers.insert (insertPos, header);
    tracks.insert  (insertPos, newTrack);

    addAndMakeVisible (header);
    addAndMakeVisible (newTrack);

    updateTrackIndexes();   // keep TrackComponent::trackIndex in sync with row/engine index
    resized();

    header->setMuted (appEngine->isTrackMuted (newTrack->getTrackIndex()));
    header->setSolo  (appEngine->isTrackSoloed (newTrack->getTrackIndex()));
    refreshSoloVisuals();

    newTrack->onRequestDeleteTrack = [this] (int uiIndex) {
        if (uiIndex >= 0 && uiIndex < tracks.size()) {
            const int engineIdx = tracks[uiIndex]->getTrackIndex();   // <-- derive now

            // Delete in engine first so indices stay consistent
            appEngine->deleteMidiTrack (engineIdx);
            //trackList->rebuildFromEngine();

            // Then remove UI rows
            removeChildComponent (headers[uiIndex]);
            removeChildComponent (tracks[uiIndex]);
            headers.remove (uiIndex);
            tracks.remove (uiIndex);

            updateTrackIndexes();
            resized();
        }
    };

    newTrack->onRequestOpenPianoRoll = [this] (int uiIndex) {
        selectedTrackIndex = uiIndex;

        const int engineIdx = tracks[uiIndex]->getTrackIndex(); // <-- derive now

        if (pianoRollWindow == nullptr) {
            pianoRollWindow = std::make_unique<PianoRollWindow> (*appEngine, engineIdx);
            pianoRollWindow->addToDesktop (pianoRollWindow->getDesktopWindowStyleFlags());
        } else if (pianoRollWindow->getTrackIndex() != engineIdx) {
            pianoRollWindow->setTrackIndex (engineIdx);
        }

        pianoRollWindow->setVisible (true);
        pianoRollWindow->toFront (true);
    };


    newTrack->onRequestOpenDrumSampler = [this] (int uiIndex) {
        if (uiIndex < 0 || uiIndex >= tracks.size()) return;

        const int engineIdx = tracks[uiIndex]->getTrackIndex(); // <-- derive now
        if (auto* eng = appEngine->getDrumAdapter (engineIdx))  // <-- pass engine idx
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

void TrackListComponent::updateTrackIndexes()
{
    for (int i = 0; i < tracks.size(); ++i)
    {
        int oldTrackIndex = tracks[i]->getTrackIndex();
        tracks[i]->setUIIndex (i);

        // In case of mismatch between indices
        if (pianoRollWindow != nullptr && oldTrackIndex == pianoRollWindow->getTrackIndex())
        {
            pianoRollWindow->setTrackIndex (i);
        }
    }
}

void TrackListComponent::refreshSoloVisuals()
{
    const bool anySolo = appEngine->anyTrackSoloed();
    for (int i = 0; i < headers.size(); ++i)
    {
        const bool thisSolo = appEngine->isTrackSoloed (tracks[i]->getTrackIndex());
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
    for (auto* h : headers) removeChildComponent (h);
    for (auto* t : tracks)  removeChildComponent (t);
    headers.clear (true);
    tracks.clear (true);

    const int n = appEngine->getNumTracks();
    for (int i = 0; i < n; ++i)
        addNewTrack(i);

    updateTrackIndexes();
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
