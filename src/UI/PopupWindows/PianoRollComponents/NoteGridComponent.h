//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef NOTEGRIDCOMPONENT_H
#define NOTEGRIDCOMPONENT_H
#include <juce_gui_basics/juce_gui_basics.h>


class NoteGridComponent : public juce::Component {
public:
    NoteGridComponent();
    ~NoteGridComponent() override;

    void paint (juce::Graphics & g) override;
    void resized () override;
private:
    juce::Array<int> blackPitches;
};



#endif //NOTEGRIDCOMPONENT_H
