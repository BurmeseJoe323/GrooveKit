#pragma once

#include "../../AppEngine/AppEngine.h"
#include "../../DatabaseManager/DatabaseManager.h"
#include "TutorialNodeComponent.h"

#include <juce_gui_basics/juce_gui_basics.h>

struct TutorialScreen;

/**
 A manager that shows either a blank canvas of clickable nodes,
 or the selected tutorial screen, with a universal Back button.
*/
class TutorialManagerComponent final : public juce::Component
{
public:
    explicit TutorialManagerComponent(AppEngine& engine, DatabaseManager& db);
    ~TutorialManagerComponent() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    /**
     Called when Back is pressed on the node view: should return to home screen.
    */
    std::function<void()> onBack;

private:
    void showNodeView();
    void showScreen(int index);



    AppEngine& appEngine;
    DatabaseManager& db;
    bool showingNodes = true;

    juce::OwnedArray<TutorialScreen> screens;
    juce::OwnedArray<TutorialNodeComponent> nodes;
    std::unique_ptr<juce::Component> currentContent;
    juce::TextButton backButton { "Back" };
    juce::Array<bool> getCompletedTutorials();
};

/**
 * Represents an individual tutorial view.
 */
struct TutorialScreen
{
    virtual ~TutorialScreen() = default;
    /** Human‑readable name for the node list. */
    [[nodiscard]] virtual juce::String getScreenName() const = 0;
    /** Returns a brand‑new component to show in the right pane. */
    virtual juce::Component* createContent() = 0;
};