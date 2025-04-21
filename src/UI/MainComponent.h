//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackView-UI/TrackEditView.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void showTrackView();
private:
    juce::TextButton openTrackView {TRANS ("OPEN TRACKVIEW")};
    std::unique_ptr<Component> view;
};
