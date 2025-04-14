#include "MainComponent.h"

#ifdef _WIN32
#include <corecrt_io.h>
#endif


#include "TrackView-UI/TrackView.h"


MainComponent::MainComponent(AppEngine& engine)
    : appEngine(engine)
{
    setSize(600, 400);

    openTrackView.onClick = [this]() { showTrackView(); };
    openTrackViewTut.onClick = [this]() {showTrackViewTutorial();};

    addAndMakeVisible(openTrackView);
    addAndMakeVisible(openTrackViewTut);

}

MainComponent::~MainComponent() = default;

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("Hello, GrooveKit!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    FlexBox box;
    box.flexDirection = FlexBox::Direction::row;
    box.justifyContent = FlexBox::JustifyContent::spaceBetween;
    box.alignItems = FlexBox::AlignItems::flexEnd;

    box.items.addArray({
        FlexItem(openTrackView)
            .withFlex(1.0f, 1.0f)
            .withMinWidth(50.0f)
            .withMinHeight(30.0f)
            .withMargin({5.0f, 10.0f, 5.0f, 10.0f}),

        FlexItem(openTrackViewTut)
            .withFlex(1.0f, 1.0f)
            .withMinWidth(50.0f)
            .withMinHeight(30.0f)
            .withMargin({5.0f, 10.0f, 5.0f, 10.0f})
    });

    box.performLayout(getLocalBounds().reduced(10));  // Add overall padding
}

void MainComponent::showTrackView() {
    trackView = std::make_unique<TrackView>(appEngine);

    // why do we have this??
    //auto view = std::make_unique<TrackView>(appEngine);

    addAndMakeVisible(trackView.get());
    trackView->setBounds(getLocalBounds());
    openTrackView.setVisible(false);
}

void MainComponent::showTrackViewTutorial() {
    trackViewTut = std::make_unique<TrackViewTut>();
    addAndMakeVisible(trackViewTut.get());

    trackViewTut->setBounds(getLocalBounds());
    openTrackView.setVisible(false);
}

    // layout child components here

