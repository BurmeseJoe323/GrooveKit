#include "MainComponent.h"
#include "InstrumentTutorial/InstrumentTutorial.h"
#include "TrackView/TrackView.h"

MainComponent::MainComponent (AppEngine& engine)
    : appEngine (engine)
{
    databaseManager.initialize();

    tutorialManager = std::make_unique<TutorialManagerComponent>(appEngine, databaseManager);
    addAndMakeVisible (tutorialManager.get());
    tutorialManager->setVisible (false);   // start hidden
    tutorialManager->onBack = [this] { showHome(); };

    openTrackView.onClick = [this] { showTrackView(); };
    addAndMakeVisible (openTrackView);

    openTutorialManager.onClick = [this] { showTutorialManager(); };
    addAndMakeVisible (openTutorialManager);

    setSize (600, 400);
}

MainComponent::~MainComponent() = default;

void MainComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    g.setColour (Colours::white);
    g.setFont (20.0f);
    g.drawText ("Hello, GrooveKit!", getLocalBounds(), Justification::centred, true);
}

void MainComponent::resized()
{
    FlexBox box;
    box.flexDirection = FlexBox::Direction::row;
    box.justifyContent = FlexBox::JustifyContent::spaceBetween;
    box.alignItems = FlexBox::AlignItems::flexEnd;

    box.items.addArray ({
        FlexItem (openTrackView)
            .withFlex (1.0f, 1.0f)
            .withMinWidth (50.0f)
            .withMinHeight (30.0f)
            .withMargin ({ 5.0f, 10.0f, 5.0f, 10.0f }),

        FlexItem (openTutorialManager)
            .withFlex (1.0f, 1.0f)
            .withMinWidth (50.0f)
            .withMinHeight (30.0f)
            .withMargin ({ 5.0f, 10.0f, 5.0f, 10.0f }) });

    box.performLayout (getLocalBounds().reduced (10)); // Add overall padding
}

void MainComponent::showTrackView()
{
    trackView = std::make_unique<TrackView> (appEngine);
    addAndMakeVisible (trackView.get());
    trackView->setBounds (getLocalBounds());

    openTrackView.setVisible (false);
    openTutorialManager.setVisible (false);
}

// void MainComponent::showInstrumentTutorial()
// {
//     instTutorial = std::make_unique<InstrumentTutorial> (databaseManager);
//
//     //Creating a safe pointer to the component
//     Component::SafePointer<MainComponent> safeThis (this);
//     instTutorial->onFinishTutorial = [safeThis]() {
//         if (auto* comp = safeThis.getComponent())
//         {
//             comp->instTutorial.reset();
//             comp->openTrackView.setVisible (true);
//             comp->openTrackViewTut.setVisible (true);
//             comp->openInstTutorial.setVisible (true);
//             comp->resized();
//         }
//     };
//
//     addAndMakeVisible (instTutorial.get());
//     instTutorial->setBounds (getLocalBounds());
//
//     openTrackView.setVisible (false);
//     openTrackViewTut.setVisible (false);
//     openInstTutorial.setVisible (false);
// }

void MainComponent::showTutorialManager()
{
    tutorialManager->setVisible (true);
    tutorialManager->setBounds (getLocalBounds());
    openTrackView.setVisible (false);
    openTutorialManager.setVisible (false);
}

void MainComponent::showHome()
{
    tutorialManager->setVisible(false);
    openTrackView.setVisible (true);
    openTutorialManager.setVisible (true);
}

