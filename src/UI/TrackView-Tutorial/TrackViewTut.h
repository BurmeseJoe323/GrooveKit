//
// Created by ikera on 4/14/2025.
//

#ifndef TRACKVIEWTUT_H
#define TRACKVIEWTUT_H

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../DatabaseManager/DatabaseManager.h"

using namespace juce;
class TrackViewTut : public Component{
public:
    TrackViewTut(DatabaseManager& dbManager);
    ~TrackViewTut() override;

    void paint(Graphics& g) override;
    void resized() override;
private:

    DatabaseManager& db;
    TextButton goBack {"Home"};
    TextButton endTutorial {"Complete Tutorial"};
    // below are examples of creating buttons and labels
    // juce::TextButton addClip {"simpl"};
    // juce::Label trackNameLabel {"Track"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackViewTut)
};


#endif //TRACKVIEWTUT_H
