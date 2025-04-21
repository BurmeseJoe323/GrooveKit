#include "MainComponent.h"

#ifdef _WIN32
#include <corecrt_io.h>
#endif


#include "TrackView-UI/TrackView.h"
#include "InstrumentTutorial/InstrumentTutorial.h"


MainComponent::MainComponent(AppEngine& engine)
    : appEngine(engine)
{
    databaseManager.initialize();
    setSize(600, 400);

    openTrackView.onClick = [this]() { showTrackView(); };
    openInstTutorial.onClick = [this]() {
        showInstrumentTutorial();
        databaseManager.addTutorial("InstrumentTutorial");
    };
    openTrackViewTut.onClick = [this]() {
        // display TrackView tutorial
        showTrackViewTutorial();
        // add Tutorial to the db.
        databaseManager.addTutorial("TrackViewTutorial");
    };

    addAndMakeVisible(openTrackView);
    addAndMakeVisible(openTrackViewTut);
    addAndMakeVisible(openInstTutorial);

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
            .withMargin({5.0f, 10.0f, 5.0f, 10.0f}),

        FlexItem(openInstTutorial)
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
    trackViewTut = std::make_unique<TrackViewTut>(databaseManager);
    addAndMakeVisible(trackViewTut.get());

    trackViewTut->setBounds(getLocalBounds());
    openTrackView.setVisible(false);
}

void MainComponent::showInstrumentTutorial() {
    instTutorial = std::make_unique<InstrumentTutorial>(databaseManager);

    //Creating a safe pointer to the component
    juce::Component::SafePointer<MainComponent> safeThis(this);
    instTutorial->onFinishTutorial = [safeThis]() {
        if (auto* comp = safeThis.getComponent()) {
            comp->instTutorial.reset();
            comp->openTrackView.setVisible(true);
            comp->openTrackViewTut.setVisible(true);
            comp->openInstTutorial.setVisible(true);
            comp->resized();
        }

    };

    addAndMakeVisible(instTutorial.get());
    instTutorial->setBounds(getLocalBounds());

    openTrackView.setVisible(false);
    openTrackViewTut.setVisible(false);
    openInstTutorial.setVisible(false);
}

    // layout child components here

