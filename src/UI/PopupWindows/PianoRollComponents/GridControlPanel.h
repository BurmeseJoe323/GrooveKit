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
    GridControlPanel (NoteGridComponent & component, GridStyleSheet & styleSheet);
    ~GridControlPanel ();

    void resized () override;
    void paint (juce::Graphics & g) override;

    void renderSequence ();
    std::function<void(int pixelsPerBar, int noteHeight)> configureGrid;

    void setQuantisation (PRE::eQuantisationValue value);
private:

    NoteGridComponent & noteGrid;
    GridStyleSheet & styleSheet;

    juce::Slider noteCompHeight, pixelsPerBar;

    juce::TextButton render;
    juce::TextButton deleteNotes;
    // juce::ToggleButton drawMIDINotes, drawMIDIText, drawVelocity;

    juce::ComboBox quantisationValue;
};


#endif //GRIDCONTROLPANEL_H
