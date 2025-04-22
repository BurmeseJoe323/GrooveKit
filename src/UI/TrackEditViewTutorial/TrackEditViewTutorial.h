#pragma once

#include "../../DatabaseManager/DatabaseManager.h"
#include "../TutorialManager/TutorialManagerComponent.h"

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;
class TrackEditViewTutorial : public Component, public TutorialScreen
{
public:
    explicit TrackEditViewTutorial (DatabaseManager& db);
    ~TrackEditViewTutorial() override;

    String getScreenName() const override
    {
        return "Track Edit View Tutorial";
    }

    Component* createContent() override
    {
        // we return a *new* instance so the manager can own it:
        return new TrackEditViewTutorial (db);
    }

    void paint (Graphics& g) override;
    void resized() override;

private:
    DatabaseManager& db;
    TextButton goBack { "Home" };
    TextButton endTutorial { "Complete Tutorial" };
    // below are examples of creating buttons and labels
    // juce::TextButton addClip {"simpl"};
    // juce::Label trackNameLabel {"Track"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackEditViewTutorial)
};
