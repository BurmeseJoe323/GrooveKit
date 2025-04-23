//
// Created by Joseph Rockwell on 4/8/25. Adapted from https://github.com/Sjhunt93/Piano-Roll-Editor/
//

#ifndef PIANOROLL_H
#define PIANOROLL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "GridControlPanel.h"
#include "KeyboardComponent.h"
#include "NoteGridComponent.h"
#include "TimelineComponent.h"

/*
 * Custom viewport that synchronizes the scrolling movement of several viewports at once
 */
class CustomViewport : public juce::Viewport {
public:
    void visibleAreaChanged(const juce::Rectangle<int> &newVisibleArea) override {
        Viewport::visibleAreaChanged(newVisibleArea);
        if (positionMoved) {
            positionMoved(getViewPositionX(), getViewPositionY());
        }
    }

    std::function<void(int, int)> positionMoved;
};

class PianoRollEditor : public juce::Component {
public:
    struct ExternalModelEditor {
        std::vector<NoteModel *> models; //const event pointers but mutable elements
        std::function<void()> update; //once you have made the edits then call this
    };

    PianoRollEditor();
    ~PianoRollEditor() override = default;

    /*
     * Called after the constructor to determine the size of the grid
     * Number of bars can be updated with updateBars
     */
    void setup (const int bars, const int pixelsPerBar, const int noteHeight);
    void updateBars (const int newNumberOfBars);

    void setScroll(double x, double y);
    void paint(juce::Graphics &g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void setPlaybackMarkerPosition (const st_int ticks, bool isVisible = true);
    void resized() override;

    PRESequence getSequence();
    void loadSequence(PRESequence sequence);

    void disableEditing(bool value);
    void setStyleSheet(GridStyleSheet styleSheet);

    void showControlPanel(bool state);

    GridControlPanel &getControlPanel();

    ExternalModelEditor getSelectedNoteModels();

    /*
     This is called when the grid is edited.
     */
    std::function<void()> onEdit;

    /*
     You can use this to implement simple MIDI synthesis when notes are being edited,
     when notes are edited this function will be called
     */
    std::function<void(int note, int velocity)> sendChange;
private:
    // Style sheet for the grid
    GridStyleSheet gridStyleSheet;

    /* The three primary components of the piano roll:
     * the keyboard visualizer, the piano roll grid,
     * and the timeline
     */
    KeyboardComponent keyboard; // Keyboard visualizer
    NoteGridComponent noteGrid; // Piano roll interface where notes will be set
    TimelineComponent timeline; // Piano roll interface where notes will be set

    // All three of the above elements are stored in a CustomViewport
    CustomViewport keyboardView, gridView, timelineView;

    // ControlPanel to change the visuals of the editor
    // NOTE: this is for development.Once a pleasing visual has been found,
    // this panel should be disposed of or adapted to new controls before merging into main
    GridControlPanel controlPanel;

    st_int  playbackTicks;
    bool    showPlaybackMarker;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollEditor)
};


#endif //PIANOROLL_H
