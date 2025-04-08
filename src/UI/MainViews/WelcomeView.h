//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H


#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

class WelcomeView : public juce::Component {
public:
    WelcomeView();
    ~WelcomeView() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    TextButton openTrackView {TRANS ("Open Track View")};
    TextButton openTrackViewTut {"TrackView Tutorial"};
    TextButton openInstTutorial {"Instrument Tutorial"};
    TextButton selectCompletedTutorials {"See completed Tutorials"};

};



#endif //WELCOMEVIEW_H
