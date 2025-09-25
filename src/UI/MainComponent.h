#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackView/TrackEditView.h"
#include "../AppEngine/AppEngine.h"

class MainComponent final : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void resized() override;

private:
    std::unique_ptr<Component> view;
    AppEngine appEngine;
};
