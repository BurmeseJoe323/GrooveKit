#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackView/TrackEditView.h"
#include "MixView/MixView.h"
#include "TransportBar/TransportBar.h"
#include "MenuBar/GrooveKitMenuBar.h"
#include "../AppEngine/AppEngine.h"

class MainComponent final : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void resized() override;

private:
    void showTrackView();
    void showMixView();

    void setView(std::unique_ptr<juce::Component> newView);

    std::unique_ptr<juce::Component> view;
    AppEngine appEngine;
    std::unique_ptr<TransportBar> transportBar;
    std::unique_ptr<GrooveKitMenuBar> menuBar;
};
