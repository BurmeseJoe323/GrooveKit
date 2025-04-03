//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::TextButton openTrackView {TRANS ("OPEN TRACKVIEW")};
    std::unique_ptr<Component> view;

    void showTrackView();
};

