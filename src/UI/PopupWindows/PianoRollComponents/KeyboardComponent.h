//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef KEYBOARDCOMPONENT_H
#define KEYBOARDCOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class KeyboardComponent : public juce::Component {
public:

    KeyboardComponent ();
    void paint (juce::Graphics & g);

private:
    std::vector<int> blackPitches;

};
#endif //KEYBOARDCOMPONENT_H
