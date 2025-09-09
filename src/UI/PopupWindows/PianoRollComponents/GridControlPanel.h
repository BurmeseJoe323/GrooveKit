//
// Created by Joseph Rockwell on 4/11/25.
//

#ifndef GRIDCONTROLPANEL_H
#define GRIDCONTROLPANEL_H

#include "GridStyleSheet.h"
#include "NoteGridComponent.h"
#include "PConstants.h"

class GridControlPanel : public juce::Component {
public:
    enum quantisationKeys {
        Quantise64 = 1,
        Quantise32,
        Quantise16,
        Quantise8,
        Quantise4,
    };

    GridControlPanel (NoteGridComponent & component, GridStyleSheet & styleSheet);
    ~GridControlPanel ();

    void resized () override;
    void paint (juce::Graphics & g) override;

    std::function<void(int pixelsPerBar, int noteHeight)> configureGrid;

private:
    // Since beats in Tracktion are quarter-notes, we will quantise in relation to that value
    const std::map<int, float> fractionsOfBeat = {
        {Quantise64, 1.f / 16.f},
        {Quantise32, 1.f / 8.f},
        {Quantise16, 1.f / 4.f},
        {Quantise8, 1.f / 2.f},
        {Quantise4, 1.f},
    };

    NoteGridComponent & noteGrid;
    GridStyleSheet & styleSheet;

    juce::Slider noteCompHeight, pixelsPerBar;

    juce::TextButton deleteNotes;
    // juce::ToggleButton drawMIDINotes, drawMIDIText, drawVelocity;

    juce::ComboBox quantisationValue;
};


#endif //GRIDCONTROLPANEL_H
