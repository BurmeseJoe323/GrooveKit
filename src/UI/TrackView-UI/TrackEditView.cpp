#include "TrackEditView.h"
#include "../../AppEngine/AppEngine.h"

TrackEditView::TrackEditView (AppEngine& engine) : appEngine (engine)
{
    trackList = std::make_unique<TrackListComponent> (appEngine);

    viewport.setScrollBarsShown (true, false); // vertical only
    viewport.setViewedComponent (trackList.get(), false);

    setupButtons();
    addAndMakeVisible (viewport);
    setSize (800, 600);
}

TrackEditView::~TrackEditView() = default;

void TrackEditView::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    g.setColour (Colours::white);
    g.setFont (20.0f);
    g.drawText ("TrackView", getLocalBounds(), Justification::centred, true);
}

void TrackEditView::resized()
{
    auto r = getLocalBounds();
    const int w = r.getWidth() / 6;
    auto topR = r.removeFromTop (30);

    newEditButton.setBounds (topR.removeFromLeft (w).reduced (2));
    playPauseButton.setBounds (topR.removeFromLeft (w).reduced (2));
    stopButton.setBounds (topR.removeFromLeft (w).reduced (2));
    recordButton.setBounds (topR.removeFromLeft (w).reduced (2));
    openEditButton.setBounds (topR.removeFromLeft (w).reduced (2));
    newTrackButton.setBounds (topR.removeFromLeft (w).reduced (2));
    deleteButton.setBounds (topR.removeFromLeft (w).reduced (2));

    viewport.setBounds (r);
}

void TrackEditView::setupButtons()
{
    newTrackButton.onClick = [this] {
        if (trackList != nullptr)
        {
            // TODO : might not be returning the right index
            int index = appEngine.addMidiTrack();
            trackList->addNewTrack (index);
        }
    };

    playPauseButton.onClick = [this] {
        appEngine.play();
    };

    stopButton.onClick = [this] { appEngine.stop(); };

    addAndMakeVisible (newEditButton);
    addAndMakeVisible (playPauseButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (recordButton);
    addAndMakeVisible (openEditButton);
    addAndMakeVisible (newTrackButton);
    addAndMakeVisible (deleteButton);
}