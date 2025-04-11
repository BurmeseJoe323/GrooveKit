//
// Created by Joseph Rockwell on 4/10/25.
//

#ifndef TIMELINECOMPONENT_H
#define TIMELINECOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>

class TimelineComponent : public juce::Component {
public:

    TimelineComponent ();
    // TODO: implement support for more time signatures
    //TimelineComponent (const timeSig);

    void setup (const int barsToDraw, const int pixelsPerBar);
    void paint (juce::Graphics & g);
    void resized ();
private:
    int barsToDraw;
    int pixelsPerBar;
};



#endif //TIMELINECOMPONENT_H
