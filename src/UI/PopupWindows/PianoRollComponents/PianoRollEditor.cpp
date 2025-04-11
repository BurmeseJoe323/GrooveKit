//
// Created by Joseph Rockwell on 4/8/25.
//

#include "PianoRollEditor.h"

PianoRollEditor::PianoRollEditor() {
    setSize(800, 600);

    // TODO: add major components to a viewport to be scrollable

    // Setup note grid
    addAndMakeVisible(noteGrid);

    // Setup keyboard
    addAndMakeVisible(keyboard);

    // Setup timeline
    addAndMakeVisible(timeline);
}

// TODO: add setup method and set bounds for all components in this method

void PianoRollEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::darkgrey.darker());
}

// void PianoRollEditorComponent::paintOverChildren (Graphics& g)
// {
//     const int x = noteGrid.getPixelsPerBar() * (playbackTicks / (4.0 * PRE::defaultResolution));
//     const int xAbsolute = viewportGrid.getViewPosition().getX();
//
//     g.setColour(Colours::greenyellow);
//     g.drawLine(x - xAbsolute, 0, x - xAbsolute, getHeight(), 5.0);
// }
// void PianoRollEditorComponent::resized()
// {
//     viewportGrid.setBounds(80, 50, getWidth()-90, controlPannel.isVisible() ? getHeight()-180 : getHeight() - 55);
//     viewportTimeline.setBounds(viewportGrid.getX(), 5, viewportGrid.getWidth()-10, viewportGrid.getY() - 5);
//     viewportPiano.setBounds(5, viewportGrid.getY(), 70, viewportGrid.getHeight()- 10);
//
//     noteGrid.setBounds(0,0,4000, 20*127);
//     noteGrid.setupGrid(900, 20, 10);
//     timelineComp.setBounds(0, 0, 100, viewportTimeline.getHeight());
//     timelineComp.setup(10, 900);
//     keyboardComp.setBounds(0, 0, viewportPiano.getWidth(), noteGrid.getHeight());
//
//     controlPannel.setBounds(5, viewportGrid.getBottom() + 5, getWidth() - 10, 140);
//
//
// }
//
// void PianoRollEditorComponent::showControlPannel (bool state)
// {
//     controlPannel.setVisible(state);
// }
// //void PianoRollEditorComponent::setStyleSheet (NoteGridStyleSheet style)
// //{
// //
// //}
// void PianoRollEditorComponent::setup (const int bars, const int pixelsPerBar, const int noteHeight)
// {
//
//     if (bars > 1 && bars < 1000) { // sensible limits..
//
//         noteGrid.setupGrid(pixelsPerBar, noteHeight, bars);
//         timelineComp.setup(bars, pixelsPerBar);
//         keyboardComp.setSize(viewportPiano.getWidth(), noteGrid.getHeight());
//     }
//     else {
//         // you might be able to have a 1000 bars but do you really need too!?
//         jassertfalse;
//     }
// }
//
// void PianoRollEditorComponent::updateBars (const int newNumberOfBars)
// {
//     if (newNumberOfBars > 1 && newNumberOfBars < 1000) { // sensible limits..
//         const float pPb = noteGrid.getPixelsPerBar();
//         const float nH = noteGrid.getNoteCompHeight();
//
//         noteGrid.setupGrid(pPb, nH, newNumberOfBars);
//         timelineComp.setup(newNumberOfBars, pPb);
//         keyboardComp.setSize(viewportPiano.getWidth(), noteGrid.getHeight());
//     }
//     else {
//         jassertfalse;
//     }
// }
//
// void PianoRollEditorComponent::loadSequence (PRESequence sequence)
// {
//     noteGrid.loadSequence(sequence);
//
//
//     // fix me, this automatically scrolls the grid
// //    const int middleNote = ((sequence.highNote - sequence.lowNote) * 0.5) + sequence.lowNote;
// //    const float scrollRatio = middleNote / 127.0;
// //    setScroll(0.0, scrollRatio);
// }
// PRESequence PianoRollEditorComponent::getSequence ()
// {
//     return noteGrid.getSequence();
// }
//
// void PianoRollEditorComponent::setScroll (double x, double y)
// {
//     viewportGrid.setViewPositionProportionately(x, y);
// }
// void PianoRollEditorComponent::setPlaybackMarkerPosition (const st_int ticks, bool isVisable)
// {
//     showPlaybackMarker = isVisable;
//     playbackTicks = ticks;
//     repaint();
//
// }
// void PianoRollEditorComponent::disableEditing (bool value)
// {
//     styleSheet.disableEditing = value;
//     noteGrid.repaint();
// }
//
// NoteGridControlPanel & PianoRollEditorComponent::getControlPannel ()
// {
//     return controlPannel;
// }
//
// PianoRollEditorComponent::ExternalModelEditor PianoRollEditorComponent::getSelectedNoteModels ()
// {
//     ExternalModelEditor mEdit;
//     mEdit.update = [this]()
//     {
//         noteGrid.resized();
//         noteGrid.repaint();
//     };
//     mEdit.models = noteGrid.getSelectedModels();
//     return mEdit;
// }