//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "TrackView-UI/TrackView.h"
class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    TextButton openTrackView {"OPEN TRACKVIEW"};
    std::unique_ptr<TrackView> trackView;

    void showTrackView();
};
