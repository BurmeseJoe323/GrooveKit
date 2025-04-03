//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TrackView : public juce::Component
{
public:
    TrackView();
    ~TrackView() override;

    void paint(juce::Graphics&) override;
    void resized() override;
};
