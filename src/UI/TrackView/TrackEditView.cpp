#include "TrackEditView.h"
#include "../../AppEngine/AppEngine.h"
#include "PopupWindows/OutputDevice/OutputDeviceWindow.h"

TrackEditView::TrackEditView(AppEngine& engine)
{
    appEngine = std::shared_ptr<AppEngine>(&engine, [](AppEngine*){});
    trackList = std::make_unique<TrackListComponent> (appEngine);

    viewport.setScrollBarsShown (true, false); // vertical only
    viewport.setViewedComponent (trackList.get(), false);

    setupButtons();
    addAndMakeVisible (viewport);
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

    viewport.setBounds (r);
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
            if (choice == 1)      index = appEngine->addInstrumentTrack();
            else if (choice == 2) index = appEngine->addDrumTrack();
            DBG("[TrackEditView] now " << appEngine->getNumTracks() << " tracks"); // insert here
            if (index >= 0) trackList->addNewTrack(index);
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
    addAndMakeVisible((outputButton));

    outputButton.onClick = [this] {
        auto* content = new OutputDeviceWindow (*appEngine);

        content->setSize (360, 140);

        auto screenBounds = outputButton.getScreenBounds();
        juce::CallOutBox::launchAsynchronously (std::unique_ptr<Component>(content), screenBounds, nullptr);


    };

    addAndMakeVisible (mixViewButton);
    mixViewButton.onClick = [this]{ if (onOpenMix) onOpenMix(); };

    addAndMakeVisible (backButton);
    backButton.onClick = [this] {
        if (onBack)
            onBack(); 
    };
}
