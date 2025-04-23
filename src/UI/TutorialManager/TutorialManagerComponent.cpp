#include "TutorialManagerComponent.h"

#include "../InstrumentTutorial/InstrumentTutorial.h"
#include "../TrackEditViewTutorial/TrackEditViewTutorial.h"

TutorialManagerComponent::TutorialManagerComponent (AppEngine& engine, DatabaseManager& db)
    : appEngine (engine), db (db)
{
    // Register tutorial screen
    screens.add (new InstrumentTutorial (db));
    screens.add (new TrackEditViewTutorial (db));

    auto completedStatus = getCompletedTutorials();

    // Create nodes
    for (int i = 0; i < screens.size(); ++i)
    {
        auto* node = new TutorialNodeComponent (screens[i]->getScreenName(), i,
            [this] (int idx) { showScreen (idx); });
        bool shouldEnable = completedStatus[i] || (i == 0) || (i > 0 && completedStatus[i - 1]);
        node->setEnabled(shouldEnable);
        nodes.add (node);
        addAndMakeVisible (node);
    }

    // Universal Back button
    addAndMakeVisible (backButton);
    backButton.onClick = [this] {
        if (!showingNodes)
            showNodeView(); // from a detail screen back to nodes
        else if (onBack)
            onBack();       // from node view back to home
    };
}

TutorialManagerComponent::~TutorialManagerComponent() = default;

void TutorialManagerComponent::paint(Graphics& g)
{
    g.fillAll (Colours::darkblue);
    g.setColour (Colours::white);
    g.setFont (20.0f);
    g.drawText ("TutorialManager", getLocalBounds(), Justification::centred, true);
}

void TutorialManagerComponent::resized()
{
    auto area = getLocalBounds().reduced (10);

    // place Back button at top-left
    auto topBar = area.removeFromTop (30);
    backButton.setBounds (topBar.removeFromLeft (80).reduced (2));

    if (showingNodes)
    {
        // Grid layout for nodes
        const int cols = 3;
        auto nodeSize = juce::jmin (150, (area.getWidth() - (cols - 1) * 20) / cols);
        for (int i = 0; i < nodes.size(); ++i)
        {
            int row = i / cols;
            int col = i % cols;
            int x = area.getX() + col * (nodeSize + 20);
            int y = area.getY() + row * (nodeSize + 20);
            nodes[i]->setBounds (x, y, nodeSize, nodeSize);
        }
    }
    else
    {
        if (auto* c = currentContent.get())
            c->setBounds (area);
    }
}

void TutorialManagerComponent::showNodeView()
{
    showingNodes = true;
    for (auto* node : nodes)
        node->setVisible (true);
    if (auto* old = currentContent.get())
        removeChildComponent (old);
    currentContent = {};
    resized();
}

void TutorialManagerComponent::showScreen (int index)
{
    showingNodes = false;
    for (auto* node : nodes)
        node->setVisible (false);
    currentContent.reset();


    auto* comp = screens[index]->createContent();
    if (auto* trackTut = dynamic_cast<TrackEditViewTutorial*>(comp))
    {
        trackTut->onTutorialCompleted = [this]() {
            showNodeView();
            auto completedStatus = getCompletedTutorials();
            for (int i = 0; i < nodes.size(); ++i)
            {
                bool shouldEnable = completedStatus[i] || (i == 0) || (i > 0 && completedStatus[i - 1]);
                nodes[i]->setEnabled(shouldEnable);
            }

        };
    }
    else if (auto* instrumentTut = dynamic_cast<InstrumentTutorial*>(comp))
    {
        instrumentTut->onFinishTutorial = [this]() {
            showNodeView();
            auto completedStatus = getCompletedTutorials();
            for (int i = 0; i < nodes.size(); ++i)
            {
                bool shouldEnable = completedStatus[i] || (i == 0) || (i > 0 && completedStatus[i - 1]);
                nodes[i]->setEnabled(shouldEnable);
            }

        };
    }
    addAndMakeVisible (comp);
    currentContent.reset(comp);
    resized();
}

juce::Array<bool> TutorialManagerComponent::getCompletedTutorials()
{
    juce::Array<bool> completed;
    for (const auto& screen : screens)
    {
        bool isDone = db.isTutorialComplete(screen->getScreenName().toStdString());
        completed.add(isDone);
    }
    return completed;
}


