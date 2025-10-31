//
// Created by Joseph Rockwell on 4/8/25. Adapted from https://github.com/Sjhunt93/Piano-Roll-Editor/
//

#ifndef PIANOROLL_H
#define PIANOROLL_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

#include "../../../AppEngine/AppEngine.h"
#include "GridControlPanel.h"
#include "KeyboardComponent.h"
#include "NoteGridComponent.h"
#include "TimelineComponent.h"

namespace te = tracktion;

/**
 * Custom viewport that synchronizes the scrolling movement of several viewports at once:
 * Scrolls keyboard and NoteGrid vertically.
 * Scrolls timeline and NoteGrid horizontally.
 */
class CustomViewport : public juce::Viewport
{
public:
    void visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea) override
    {
        Viewport::visibleAreaChanged (newVisibleArea);
        if (positionMoved)
        {
            positionMoved (getViewPositionX(), getViewPositionY());
        }
    }

    std::function<void (int, int)> positionMoved;
};

class PianoRollEditor : public juce::Component
{
public:
    PianoRollEditor (AppEngine& engine, te::MidiClip* clip);
    PianoRollEditor (AppEngine& engine, int trackIndex);
    ~PianoRollEditor() override = default;

    // Change the currently viewed/edited clip
    void setClip (te::MidiClip* clip);

    /*
     * Called after the constructor to determine the size of the grid
     * Number of bars can be updated with updateBars
     */
    void setup (const int bars, const int pixelsPerBar, const int noteHeight);
    void updateBars (const int newNumberOfBars);

    void setScroll (double x, double y);
    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void setPlaybackMarkerPosition (const st_int ticks, bool isVisible = true);
    void resized() override;

    const te::MidiList& getSequence();
    // void loadSequence();

    void setStyleSheet (GridStyleSheet styleSheet);

    void showControlPanel (bool state);
    bool keyPressed (const juce::KeyPress& key) override;

    GridControlPanel& getControlPanel();

    std::function<void()> onClose;

    /*
     This is called when the grid is edited.
     */
    std::function<void()> onEdit;

    /*
     You can use this to implement simple MIDI synthesis when notes are being edited,
     when notes are edited this function will be called
     */
    std::function<void (int note, int velocity)> sendChange;

private:
    // Adjustable parameters for pixels per bar and pixels per note
    float pixelsPerBar = 550, noteHeight = 15;

    // Number of bars to draw
    int numBars = 10;

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

    st_int playbackTicks;
    bool showPlaybackMarker;

    juce::TextButton closeButton { "x" };
};
#endif