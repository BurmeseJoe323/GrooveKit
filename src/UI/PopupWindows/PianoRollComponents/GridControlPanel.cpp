//
// Created by Joseph Rockwell on 4/11/25.
//

#include "GridControlPanel.h"
#include "GridStyleSheet.h"
#include "NoteGridComponent.h"
#include "PConstants.h"

//==============================================================================
// Lifecycle
//==============================================================================

GridControlPanel::GridControlPanel(NoteGridComponent &component, GridStyleSheet &ss) : noteGrid(component),
    styleSheet(ss) {
    // TODO : figure out how and if we want to resize the grid ?


    // addAndMakeVisible(drawMIDINotes);
    // addAndMakeVisible(drawMIDIText);
    // addAndMakeVisible(drawVelocity);
    addAndMakeVisible(deleteNotes);

    deleteNotes.setButtonText("Delete notes");
    // drawMIDINotes.setButtonText("Draw MIDI Notes");
    // drawMIDIText.setButtonText("Draw MIDI Text");
    // drawVelocity.setButtonText("Draw Velocity");

    deleteNotes.onClick = [this]() {
        noteGrid.deleteAllSelected();
    };

    // drawMIDIText.onClick = [this]() {
    //     styleSheet.drawMIDINoteStr = drawMIDIText.getToggleState();
    //     styleSheet.drawMIDINum = drawMIDINotes.getToggleState();
    //     styleSheet.drawVelocity = drawVelocity.getToggleState();
    //     noteGrid.repaint();
    // };
    // drawVelocity.onClick = drawMIDINotes.onClick = drawMIDIText.onClick;

    addAndMakeVisible(quantisationValue);
    quantisationValue.addItem("1/64", Quantise64);
    quantisationValue.addItem("1/32", Quantise32);
    quantisationValue.addItem("1/16", Quantise16);
    quantisationValue.addItem("1/8", Quantise8);
    quantisationValue.addItem("1/4", Quantise4);
    quantisationValue.setSelectedId(Quantise16);

    quantisationValue.onChange = [this]() {
        noteGrid.setQuantisation(fractionsOfBeat.at(quantisationValue.getSelectedId()));
    };
}

GridControlPanel::~GridControlPanel() {
}

//==============================================================================
// Component Overrides
//==============================================================================

void GridControlPanel::resized() {

    // drawMIDINotes.setBounds(pixelsPerBar.getRight() + 5, 5, 150, (getHeight() / 3) - 10);
    // drawMIDIText.setBounds(pixelsPerBar.getRight() + 5, drawMIDINotes.getBottom() + 5, 200, drawMIDINotes.getHeight());
    // drawVelocity.setBounds(pixelsPerBar.getRight() + 5, drawMIDIText.getBottom() + 5, 200, drawMIDINotes.getHeight());

    quantisationValue.setBounds(getWidth() - 250, 5, 200, 40);
    deleteNotes.setBounds(quantisationValue.getX() - 150, 5, 120, quantisationValue.getHeight());
}

void GridControlPanel::paint(juce::Graphics &g) {
}