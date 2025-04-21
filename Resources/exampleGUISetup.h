//
// Created by ikera on 4/8/2025.
//

#ifndef EXAMPLEGUISETUP_H
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class exampleGuiSetup : public juce::Component{
public:
    exampleGuiSetup();
    ~exampleGuiSetup();

    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    // below are examples of creating buttons and labels
    // juce::TextButton addClip {"simpl"};
    // juce::Label trackNameLabel {"Track"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (exampleGuiSetup)
};


#endif //EXAMPLEGUISETUP_H
