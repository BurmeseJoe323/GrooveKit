//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef KEYBOARDCOMPONENT_H
#define KEYBOARDCOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <set>

//==============================================================================
/**
 * @brief Visual piano keyboard display for the MIDI piano roll editor.
 *
 * KeyboardComponent renders a vertical piano keyboard on the left side of the
 * piano roll editor, providing visual reference for MIDI note pitches. The keyboard
 * displays alternating white and black keys with proper chromatic spacing.
 *
 * **Features:**
 * - Renders 128 MIDI notes (0-127) as piano keys
 * - Black keys (sharps/flats) rendered with different styling
 * - Vertical orientation with higher pitches at the top
 * - Synchronized with NoteGridComponent note height
 * - Visual pitch reference for MIDI editing
 *
 * **Coordinate System:**
 * - Y-axis: Top = highest pitch (MIDI 127), Bottom = lowest pitch (MIDI 0)
 * - Each key height matches the note height in the grid
 * - Black keys are rendered with offset/indentation
 *
 * **Integration:**
 * Used by PianoRollEditor as the left-side reference panel. Height and note spacing
 * must match NoteGridComponent for proper alignment.
 *
 * @see PianoRollEditor
 * @see NoteGridComponent
 * @see PConstants::pitches_names
 */
class KeyboardComponent : public juce::Component
{
public:
    //==============================================================================
    /** @brief Constructs the keyboard component and initializes black key pitches. */
    KeyboardComponent();

    //==============================================================================
    // Component Overrides

    /**
     * @brief Renders the piano keyboard with white and black keys.
     *
     * Draws all 128 MIDI notes as piano keys with proper chromatic coloring.
     * Black keys (C#, D#, F#, G#, A#) are drawn with darker styling and offset.
     *
     * @param g Graphics context for rendering
     */
    void paint (juce::Graphics& g) override;

    //==============================================================================
    // Drum Track Support (Written by Claude Code)

    /**
     * @brief Sets whether this keyboard should dim non-drum notes.
     *
     * When true, MIDI notes outside 36-51 are dimmed to indicate they're not used.
     *
     * @param isDrum True if editing a drum track
     */
    void setIsDrumTrack (bool isDrum) { isDrumTrack = isDrum; repaint(); }

    /**
     * @brief Sets the note component height to match the note grid.
     *
     * @param height Height in pixels per MIDI note (must match NoteGridComponent)
     */
    void setNoteHeight (float height) { noteHeight = height; repaint(); }

private:
    //==============================================================================
    // Private Members

    juce::Array<int> blackPitches; ///< Pitch classes for black keys (1=C#, 3=D#, 6=F#, 8=G#, 10=A#)
    bool isDrumTrack = false;      ///< True if editing a drum track (dims non-drum notes). (Written by Claude Code)
    float noteHeight = 20.0f;      ///< Height per MIDI note, synchronized with NoteGridComponent. (Written by Claude Code)
};

#endif //KEYBOARDCOMPONENT_H
