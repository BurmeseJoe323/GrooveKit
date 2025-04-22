#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 A clickable node representation for a tutorial screen.
*/
class TutorialNodeComponent : public juce::Component
{
public:
    /**
     Constructs a node with a display name and an index.
     onNodeClicked will be called with the index when clicked.
    */
    TutorialNodeComponent(const juce::String& name, int idx, std::function<void(int)> onNodeClicked)
      : title(name), index(idx), callback(std::move(onNodeClicked))
    {
        setInterceptsMouseClicks(true, true);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colours::darkgrey);
        g.fillEllipse(bounds);
        g.setColour(juce::Colours::white);
        g.drawText(title, getLocalBounds(), juce::Justification::centred);
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        if (callback)
            callback(index);
    }

    void resized() override {}

private:
    juce::String title;
    int index;
    std::function<void(int)> callback;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TutorialNodeComponent)
};