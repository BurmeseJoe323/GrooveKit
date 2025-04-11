//
// Created by Joseph Rockwell on 4/8/25.
//

#include "PianoRollEditor.h"

PianoRollEditor::PianoRollEditor() : noteGrid(gridStyleSheet), controlPanel(noteGrid, gridStyleSheet) {
    // setVisible(true);

    // TODO: Check if the viewport sizes are being set anywhere
    // Setup note grid
    addAndMakeVisible(gridView);
    gridView.setViewedComponent(&noteGrid, false);
    gridView.setScrollBarsShown(true, true);
    gridView.setScrollBarThickness(10);

    // Setup timeline
    // addAndMakeVisible(timelineView);
    // timelineView.setViewedComponent(&timeline, false);
    // timelineView.setScrollBarsShown(false, false);

    // Setup keyboard
    addAndMakeVisible(keyboardView);
    keyboardView.setViewedComponent(&keyboard, false);
    keyboardView.setScrollBarsShown(false, false);

    // Scroll the other components when the grid is scrolled
    gridView.positionMoved = [this](int x, int y) {
        timelineView.setViewPosition(x, y);
        keyboardView.setViewPosition(x, y);
    };

    // Setup control panel
    addAndMakeVisible(controlPanel);
    controlPanel.configureGrid = [this](int pixelsPerBar, int noteHeight) {
        setup(10, pixelsPerBar, noteHeight);
    };

    playbackTicks = 0;
    showPlaybackMarker = false;
}

void PianoRollEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::darkgrey.darker());
}

void PianoRollEditor::resized() {
}

void PianoRollEditor::showControlPanel(bool state) {
   controlPanel.setVisible(state);
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
//     viewportGrid.setBounds(80, 50, getWidth()-90, controlPanel.isVisible() ? getHeight()-180 : getHeight() - 55);
//     viewportTimeline.setBounds(viewportGrid.getX(), 5, viewportGrid.getWidth()-10, viewportGrid.getY() - 5);
//     viewportPiano.setBounds(5, viewportGrid.getY(), 70, viewportGrid.getHeight()- 10);
//
//     noteGrid.setBounds(0,0,4000, 20*127);
//     noteGrid.setupGrid(900, 20, 10);
//     timelineComp.setBounds(0, 0, 100, viewportTimeline.getHeight());
//     timelineComp.setup(10, 900);
//     keyboardComp.setBounds(0, 0, viewportPiano.getWidth(), noteGrid.getHeight());
//
//     controlPanel.setBounds(5, viewportGrid.getBottom() + 5, getWidth() - 10, 140);
//
//
// }
//
// void PianoRollEditorComponent::showControlPanel (bool state)
// {
//     controlPanel.setVisible(state);
// }
// //void PianoRollEditorComponent::setStyleSheet (NoteGridStyleSheet style)
// //{
// //
// //}

void PianoRollEditor::setup (const int bars, const int pixelsPerBar, const int noteHeight)
{
    // NOTE: there's probably a better way to do this. Depending on how we implement bars, we may not
    // need to do this check at all
    if (bars > 1 && bars < 1000) {
        noteGrid.setupGrid(pixelsPerBar, noteHeight, bars);
        timeline.setup(bars, pixelsPerBar);
        keyboard.setSize(keyboardView.getWidth(), noteGrid.getHeight());
    }
    else {
        jassertfalse;
    }
}


void PianoRollEditor::updateBars(const int newNumberOfBars) {
    if (newNumberOfBars > 1 && newNumberOfBars < 1000) { // sensible limits..
        const float pixelsPerBar = noteGrid.getPixelsPerBar();
        const float noteHeight = noteGrid.getNoteCompHeight();

        noteGrid.setupGrid(pixelsPerBar, noteHeight, newNumberOfBars);
        timeline.setup(newNumberOfBars, pixelsPerBar);
        keyboard.setSize(keyboardView.getWidth(), noteGrid.getHeight());
    }
    else {
        jassertfalse;
    }
}

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

void PianoRollEditor::setPlaybackMarkerPosition(const st_int ticks, bool isVisible) {
    showPlaybackMarker = isVisible;
    playbackTicks = ticks;
    repaint();
}

// void PianoRollEditorComponent::disableEditing (bool value)
// {
//     styleSheet.disableEditing = value;
//     noteGrid.repaint();
// }
//
// NoteGridControlPanel & PianoRollEditorComponent::getControlPanel ()
// {
//     return controlPanel;
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