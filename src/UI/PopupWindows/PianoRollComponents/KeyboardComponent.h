//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef KEYBOARDCOMPONENT_H
#define KEYBOARDCOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <set>

class KeyboardComponent : public juce::Component {
public:

    KeyboardComponent ();
    void paint (juce::Graphics & g) override;

private:
    juce::Array<int> blackPitches;

};
#endif //KEYBOARDCOMPONENT_H
