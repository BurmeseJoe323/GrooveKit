//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef PIANOROLLWINDOW_H
#define PIANOROLLWINDOW_H
#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>


class PianoRollWindow : public juce::DocumentWindow {
public:
    PianoRollWindow();
    ~PianoRollWindow() override;

    void closeButtonPressed() override;
    class PianoRoll : public juce::Component {
    public:
        PianoRoll() {}
        ~PianoRoll() override = default;
        void paint(juce::Graphics& g) override {
            g.setColour(juce::Colours::darkseagreen);
        }
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRoll)
    };
private:
    std::unique_ptr<PianoRoll> pianoRoll;
};



#endif //PIANOROLLWINDOW_H
