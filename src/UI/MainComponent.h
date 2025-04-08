//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../AppEngine/AppEngine.h"
class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void showTrackView();
    void showAppView();
private:
    std::unique_ptr<Component> view;
};
