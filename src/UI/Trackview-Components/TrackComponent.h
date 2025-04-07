//
// Created by ikera on 4/6/2025.
//
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;
class TrackComponent : public juce::Component {
  public:
    TrackComponent();
    ~TrackComponent();

    void paint(Graphics&);
    void resized();
};
