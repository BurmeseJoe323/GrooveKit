#include "TutorialManagerComponent.h"
#include "../TrackEditViewTutorial/TrackEditViewTutorial.h"

TutorialManagerComponent::TutorialManagerComponent(AppEngine& engine, DatabaseManager& db)
  : appEngine(engine), db(db)
{
    // Register your tutorial screens
    screens.add(new TrackEditViewTutorial(db));
    // Add more: screens.add(new OtherTutorial());

    // Create nodes
    for (int i = 0; i < screens.size(); ++i)
    {
        auto* node = new TutorialNodeComponent(screens[i]->getName(), i,
            [this](int idx) { showScreen(idx); });
        nodes.add(node);
        addAndMakeVisible(node);
    }

    // Universal Back Button
    addAndMakeVisible(backButton);
    backButton.onClick = [this]
    {
        if (! showingNodes)
            showNodeView();       // from a detail screen back to nodes
        else if (onBack)        
            onBack();             // from node view back to home
    };
}

TutorialManagerComponent::~TutorialManagerComponent() = default;

void TutorialManagerComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    // place Back button at top-left
    auto topBar = area.removeFromTop(30);
    backButton.setBounds(topBar.removeFromLeft(80).reduced(2));

    if (showingNodes)
    {
        // Grid layout for nodes
        const int cols = 3;
        auto nodeSize = juce::jmin(150, (area.getWidth() - (cols-1)*20) / cols);
        for (int i = 0; i < nodes.size(); ++i)
        {
            int row = i / cols;
            int col = i % cols;
            int x = area.getX() + col * (nodeSize + 20);
            int y = area.getY() + row * (nodeSize + 20);
            nodes[i]->setBounds(x, y, nodeSize, nodeSize);
        }
    }
    else
    {
        if (auto* c = currentContent.getComponent())
            c->setBounds(area);
    }
}

void TutorialManagerComponent::showNodeView()
{
    showingNodes = true;
    for (auto* node : nodes)
        node->setVisible(true);
    if (auto* old = currentContent.getComponent())
        removeChildComponent(old);
    currentContent = {};
    resized();
}

void TutorialManagerComponent::showScreen(int index)
{
    showingNodes = false;
    for (auto* node : nodes)
        node->setVisible(false);

    auto* comp = screens[index]->createContent();
    addAndMakeVisible(comp);
    currentContent = comp;
    resized();
}