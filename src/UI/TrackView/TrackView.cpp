#include "TrackView.h"
#include "../../AppEngine/AppEngine.h"

TrackView::TrackView (AppEngine& engine) : appEngine (engine)
{
    addAndMakeVisible (newTrackButton);
    newTrackButton.onClick = [this] {
        if (editComponent != nullptr)
        {
            int index = appEngine.addMidiTrack();
            editComponent->addNewTrack (index);
        }
    };

    addAndMakeVisible (playPauseButton);
    playPauseButton.onClick = [this] { appEngine.play(); };

    addAndMakeVisible (stopButton);
    stopButton.onClick = [this] { appEngine.stop(); };

    addAndMakeVisible (backButton);
    backButton.onClick = [this] {
        if (onBack)
            onBack(); // from track view back to home
    };

    addAndMakeVisible (newEditButton);
    addAndMakeVisible (recordButton);
    addAndMakeVisible (openEditButton);

    editComponent = std::make_unique<EditComponent> (appEngine);
    addAndMakeVisible (editComponent.get());

    setSize (800, 600);
}

TrackView::~TrackView() = default;

void TrackView::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    g.setColour (Colours::white);
    g.setFont (20.0f);
    g.drawText ("TrackView", getLocalBounds(), Justification::centred, true);
}

void TrackView::resized()
{
    auto r = getLocalBounds();
    int w = r.getWidth() / 7;
    auto topR = r.removeFromTop (30);
    backButton.setBounds (topR.removeFromLeft (w).reduced (2));
    newEditButton.setBounds (topR.removeFromLeft (w).reduced (2));
    openEditButton.setBounds (topR.removeFromLeft (w).reduced (2));
    playPauseButton.setBounds (topR.removeFromLeft (w).reduced (2));
    stopButton.setBounds (topR.removeFromLeft (w).reduced (2));
    recordButton.setBounds (topR.removeFromLeft (w).reduced (2));
    newTrackButton.setBounds (topR.removeFromLeft (w).reduced (2));

    if (editComponent != nullptr)
        editComponent->setBounds (r);
}
