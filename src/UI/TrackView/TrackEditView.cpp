#include "TrackEditView.h"
#include "../../AppEngine/AppEngine.h"
#include "PopupWindows/OutputDevice/OutputDeviceWindow.h"

TrackEditView::TrackEditView (AppEngine& engine)
{
    appEngine = std::shared_ptr<AppEngine> (&engine, [] (AppEngine*) {});
    trackList = std::make_unique<TrackListComponent> (appEngine);

    trackList->setPixelsPerSecond (pixelsPerSecond);
    trackList->setViewStart (viewStart);

    viewport.setScrollBarsShown (true, false); // vertical only
    viewport.setViewedComponent (trackList.get(), false);

    setupButtons();
    addAndMakeVisible (viewport);

    // Initialize and hide the piano roll editor
    pianoRoll = std::make_unique<PianoRollEditor>(*appEngine, -1);
    addAndMakeVisible(pianoRoll.get());
    pianoRoll->setVisible(false);

    // Split the view vertically
    verticalLayout.setItemLayout(0, -0.45, -0.85, -0.6); // Track list takes 70%
    verticalLayout.setItemLayout(1, 5, 5, 5);          // 5-pixel splitter
    verticalLayout.setItemLayout(2, -0.15, -0.55, -0.4); // Piano roll takes 30%

    // Create and add resizer bar (index 1 in components array)
    resizerBar = std::make_unique<PianoRollResizerBar>(&verticalLayout, 1, false);
    addAndMakeVisible(resizerBar.get());

}

TrackEditView::~TrackEditView() = default;

void TrackEditView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawText ("TrackView", getLocalBounds(), juce::Justification::centred, true);
}

void TrackEditView::resized()
{
    auto r = getLocalBounds();
    const int w = r.getWidth() / 7;
    auto topR = r.removeFromTop (30);

    backButton.setBounds (topR.removeFromLeft (w).reduced (2));
    //newEditButton.setBounds (topR.removeFromLeft (w).reduced (2));
    //openEditButton.setBounds (topR.removeFromLeft (w).reduced (2));
    playPauseButton.setBounds (topR.removeFromLeft (w).reduced (2));
    stopButton.setBounds (topR.removeFromLeft (w).reduced (2));
    recordButton.setBounds (topR.removeFromLeft (w).reduced (2));
    newTrackButton.setBounds (topR.removeFromLeft (w).reduced (2));
    outputButton.setBounds (topR.removeFromLeft (w).reduced (2));
    mixViewButton.setBounds (topR.removeFromLeft (w).reduced (2));

    if (pianoRoll->isVisible())
    {
        juce::Component *comps[] = { &viewport, resizerBar.get(), pianoRoll.get() };
        verticalLayout.layOutComponents(comps, 3, r.getX(), r.getY(), r.getWidth(), r.getHeight(), true, true);
    }
    else
    {
        viewport.setBounds (r);
    }
}

void TrackEditView::setupButtons()
{
    newTrackButton.onClick = [this] {
        juce::PopupMenu m;
        m.addItem (1, "Instrument (FourOsc)");
        m.addItem (2, "Drum (Sampler)");

        m.showMenuAsync (juce::PopupMenu::Options(), [this] (int choice) {
            if (!trackList || choice == 0)
                return;

            int index = -1;
            if (choice == 1)
                index = appEngine->addInstrumentTrack();
            else if (choice == 2)
                index = appEngine->addDrumTrack();
            DBG ("[TrackEditView] now " << appEngine->getNumTracks() << " tracks"); // insert here
            if (index >= 0)
            {
                trackList->addNewTrack (index);
                trackList->setPixelsPerSecond (pixelsPerSecond);
                trackList->setViewStart (viewStart);
            }
        });
    };

    playPauseButton.onClick = [this] {
        appEngine->play();
    };

    stopButton.onClick = [this] { appEngine->stop(); };

    addAndMakeVisible (newEditButton);
    addAndMakeVisible (playPauseButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (recordButton);
    addAndMakeVisible (openEditButton);
    addAndMakeVisible (newTrackButton);
    addAndMakeVisible ((outputButton));

    outputButton.onClick = [this] {
        auto* content = new OutputDeviceWindow (*appEngine);

        content->setSize (360, 140);

        auto screenBounds = outputButton.getScreenBounds();
        juce::CallOutBox::launchAsynchronously (std::unique_ptr<Component> (content), screenBounds, nullptr);
    };

    addAndMakeVisible (mixViewButton);
    mixViewButton.onClick = [this] { if (onOpenMix) onOpenMix(); };

    addAndMakeVisible (backButton);
    backButton.onClick = [this] {
        if (onBack)
            onBack();
    };
}

void TrackEditView::showPianoRoll(int trackIndex)
{
    if (pianoRollTrackIndex != trackIndex)
    {
        pianoRollTrackIndex = trackIndex;
        pianoRoll = std::make_unique<PianoRollEditor> (*appEngine, trackIndex);
        addAndMakeVisible (pianoRoll.get());
    }
    pianoRoll->setVisible (true);
    resized();
}

void TrackEditView::hidePianoRoll()
{
    pianoRoll->setVisible (false);
    pianoRollTrackIndex = -1;
    resized();
}

void TrackEditView::PianoRollResizerBar::hasBeenMoved()
{
    DBG("X: " << this->getX() << " Y: " << this->getY());
    resized();
}

void TrackEditView::PianoRollResizerBar::mouseDrag (const juce::MouseEvent& event)
{
    DBG("X: " << this->getX() << " Y: " << this->getY());
    // this->setTopLeftPosition (this->getX(), event.getPosition().getY());
    hasBeenMoved();
}

 TrackEditView::PianoRollResizerBar::PianoRollResizerBar (juce::StretchableLayoutManager* layoutToUse, int itemIndexInLayout, bool isBarVertical) : StretchableLayoutResizerBar(layoutToUse, itemIndexInLayout, isBarVertical)
{
}

TrackEditView::PianoRollResizerBar::~PianoRollResizerBar()
= default;
