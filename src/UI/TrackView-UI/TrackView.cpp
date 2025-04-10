#include "TrackView.h"
#include "../../AppEngine/AppEngine.h"

TrackView::TrackView()
{
    // playButton.onClick = [this]() { engine.play(); };
    // stopButton.onClick = [this]() { engine.stop(); };
    appEngine = std::make_shared<AppEngine>();

    newTrackButton.onClick = [this]() {
        if (editComponent != nullptr) {
            int index = appEngine->addMidiTrack();
            editComponent->addNewTrack(index);
        }
    };

    playPauseButton.onClick = [this]() {
        appEngine->play();
    };
    stopButton.onClick = [this]() {appEngine->stop();};

    addAndMakeVisible(newEditButton);
    addAndMakeVisible(playPauseButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(recordButton);
    addAndMakeVisible(openEditButton);
    addAndMakeVisible(newTrackButton);
    addAndMakeVisible(deleteButton);

    editComponent = std::make_unique<EditComponent>(appEngine);
    addAndMakeVisible(editComponent.get());
    setSize(800, 600);
}

TrackView::~TrackView() = default;

void TrackView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("TrackView", getLocalBounds(), juce::Justification::centred, true);
}

void TrackView::resized()
{
    auto r = getLocalBounds();
    int w = r.getWidth() / 6;
    auto topR = r.removeFromTop(30);
    newEditButton.setBounds(topR.removeFromLeft(w).reduced(2));
    playPauseButton.setBounds(topR.removeFromLeft(w).reduced(2));
    stopButton.setBounds(topR.removeFromLeft(w).reduced(2));
    recordButton.setBounds(topR.removeFromLeft(w).reduced(2));
    openEditButton.setBounds(topR.removeFromLeft(w).reduced(2));
    newTrackButton.setBounds(topR.removeFromLeft(w).reduced(2));
    deleteButton.setBounds(topR.removeFromLeft(w).reduced(2));

    if (editComponent != nullptr)
        editComponent->setBounds(r);
}




