//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef PIANOROLL_H
#define PIANOROLL_H

#include <juce_gui_basics/juce_gui_basics.h>
class PianoRoll : public juce::Component {
public:
    PianoRoll();

    ~PianoRoll() override = default;

    void paint(juce::Graphics &g) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRoll)
};


#endif //PIANOROLL_H
