#include "TrackListComponent.h"
#include "DrumSamplerView/DrumSamplerLauncher.h"
#include "DrumSamplerView/DrumSamplerView.h"
#include <juce_graphics/fonts/harfbuzz/hb-ot-head-table.hh>

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

    const int headerWidth = 140;
    const int trackHeight = 100;
    const int margin = 2;
    const int addButtonSpace = 30;

    int numTracks = headers.size();
    int contentH = numTracks * trackHeight + addButtonSpace;

    // Set the size to either default to the parent's height if the content height isn't tall enough
    setSize (getParentWidth(), contentH > getParentHeight() ? contentH : getParentHeight());

    auto bounds = getLocalBounds();
    bounds.removeFromBottom (addButtonSpace); // Space for add button
    for (int i = 0; i < numTracks; i++)
    {
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
    // Select random color from palette
    auto newColor = trackColors[tracks.size() % trackColors.size()];

    auto* header = new TrackHeaderComponent();
    auto* newTrack = new TrackComponent (appEngine, engineIdx, newColor);
    newTrack->setEngineIndex (engineIdx);

    newTrack->setPixelsPerSecond (100.0);
    newTrack->setViewStart (0s);

    header->addListener (newTrack);

    // Set the track name on the header
    const bool isDrum = appEngine->isDrumTrack (newTrack->getEngineIndex());
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

    header->setMuted (appEngine->isTrackMuted (newTrack->getEngineIndex()));
    header->setSolo (appEngine->isTrackSoloed (newTrack->getEngineIndex()));
    refreshSoloVisuals();

    newTrack->onRequestDeleteTrack = [this] (int uiIndex) {
        if (uiIndex >= 0 && uiIndex < tracks.size())
        {
            const int engineIdxToDelete = tracks[uiIndex]->getEngineIndex();
            removeChildComponent (headers[uiIndex]);
            removeChildComponent (tracks[uiIndex]);
            headers.remove (uiIndex);
            tracks.remove (uiIndex);
            appEngine->deleteMidiTrack (engineIdxToDelete);
            updateTrackIndexes();
            resized();
        }
    };

    newTrack->onRequestOpenPianoRoll = [this] (int uiIndex) {
        if (uiIndex >= 0 && uiIndex < tracks.size())
        {
            int engineIdx = tracks[uiIndex]->getEngineIndex();
            selectedTrackIndex = engineIdx;

            // If the window doesn't exist, create it.
            if (pianoRollWindow == nullptr)
            {
                pianoRollWindow = std::make_unique<PianoRollWindow> (*appEngine, engineIdx);
                pianoRollWindow->addToDesktop (pianoRollWindow->getDesktopWindowStyleFlags());
            }

            // If it's a different track, set the new track index and update the editor
            if (pianoRollWindow->getTrackIndex() != engineIdx)
            {
                pianoRollWindow->setTrackIndex (engineIdx); // You will need to implement this function
            }

            pianoRollWindow->setVisible (true);
            pianoRollWindow->toFront (true);
        }
    };

    newTrack->onRequestOpenDrumSampler = [this] (int uiIndex) {
        if (uiIndex < 0 || uiIndex >= tracks.size())
            return;

        const int engineIdx = tracks[uiIndex]->getEngineIndex();

        if (auto* eng = appEngine->getDrumAdapter (engineIdx))
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
        tracks[i]->setTrackIndex (i);
}

void TrackListComponent::refreshSoloVisuals()
{
    const bool anySolo = appEngine->anyTrackSoloed();
    for (int i = 0; i < headers.size(); ++i)
    {
        const bool thisSolo = appEngine->isTrackSoloed (tracks[i]->getEngineIndex());
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
