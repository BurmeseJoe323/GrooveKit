//
// Created by Joseph Rockwell on 4/11/25.
//

#include "GridControlPanel.h"
#include "GridStyleSheet.h"
#include "PConstants.h"

GridControlPanel::GridControlPanel(NoteGridComponent &component, GridStyleSheet &ss) : noteGrid(component),
    styleSheet(ss) {
    addAndMakeVisible(noteCompHeight);
    addAndMakeVisible(pixelsPerBar);

    // These are all arbitary values
    noteCompHeight.setRange(10, 30, 1);
    pixelsPerBar.setRange(400, 2000, 1);

    pixelsPerBar.setTextValueSuffix(" Pixels per bar");
    noteCompHeight.setTextValueSuffix(" Pixels per row");


    pixelsPerBar.setValue(900, juce::dontSendNotification);
    noteCompHeight.setValue(20, juce::dontSendNotification);

    pixelsPerBar.onValueChange = [this]() {
        if (configureGrid) {
            configureGrid(pixelsPerBar.getValue(), noteCompHeight.getValue());
        }
    };
    noteCompHeight.onValueChange = pixelsPerBar.onValueChange;


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
    quantisationValue.addItem("1/64", PRE::eQuantisationValueNone + 1);
    quantisationValue.addItem("1/32", PRE::eQuantisationValue1_32 + 1);
    quantisationValue.addItem("1/16", PRE::eQuantisationValue1_16 + 1);
    quantisationValue.addItem("1/8", PRE::eQuantisationValue1_8 + 1);
    quantisationValue.setSelectedItemIndex(1);

    quantisationValue.onChange = [this]() {
        noteGrid.setQuantisation(quantisationValue.getSelectedItemIndex());
    };
}

GridControlPanel::~GridControlPanel() {
}

void GridControlPanel::setQuantisation(PRE::eQuantisationValue value) {
    quantisationValue.setSelectedItemIndex(value);
}

void GridControlPanel::resized() {
    pixelsPerBar.setBounds(5, 5, 300, (getHeight() / 2) - 20);
    noteCompHeight.setBounds(5, pixelsPerBar.getBottom() + 5, 300, (getHeight() / 2) - 20);

    pixelsPerBar.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, 150, pixelsPerBar.getHeight() - 5);
    noteCompHeight.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, 150,
                                   noteCompHeight.getHeight() - 5);


    // drawMIDINotes.setBounds(pixelsPerBar.getRight() + 5, 5, 150, (getHeight() / 3) - 10);
    // drawMIDIText.setBounds(pixelsPerBar.getRight() + 5, drawMIDINotes.getBottom() + 5, 200, drawMIDINotes.getHeight());
    // drawVelocity.setBounds(pixelsPerBar.getRight() + 5, drawMIDIText.getBottom() + 5, 200, drawMIDINotes.getHeight());

    quantisationValue.setBounds(getWidth() - 250, 5, 200, 40);
    deleteNotes.setBounds(quantisationValue.getX() - 150, 5, 120, quantisationValue.getHeight());
}

void GridControlPanel::paint(juce::Graphics &g) {
}