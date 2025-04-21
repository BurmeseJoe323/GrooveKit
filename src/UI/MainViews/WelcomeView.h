//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H


#include <juce_gui_basics/juce_gui_basics.h>
class WelcomeView : public juce::Component {
public:
    WelcomeView();
    ~WelcomeView() override;

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::TextButton openTrackView {TRANS ("Open Track Edit View")};
};



#endif //WELCOMEVIEW_H
