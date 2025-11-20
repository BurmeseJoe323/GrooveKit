//
// Created by Joseph Rockwell on 4/11/25.
//

#ifndef GRIDSTYLESHEET_H
#define GRIDSTYLESHEET_H

//==============================================================================
/**
 * @brief Style configuration for piano roll grid display options.
 *
 * GridStyleSheet manages visual display preferences for the piano roll editor,
 * controlling whether MIDI note numbers, note names, and velocity values are
 * rendered on NoteComponents. This lightweight configuration object is shared
 * between GridControlPanel (which modifies settings) and NoteComponent (which
 * reads settings for rendering).
 *
 * **Features:**
 * - Toggle MIDI note numbers (0-127) on notes
 * - Toggle note name strings (C, C#, D, etc.) on notes
 * - Toggle velocity values (0-127) on notes
 * - Controlled modification via friend class pattern
 *
 * **Design Pattern:**
 * Uses the friend class pattern to allow GridControlPanel direct write access
 * to private members while providing read-only access to other classes via
 * getter methods. This avoids boilerplate setters while maintaining encapsulation.
 *
 * **Integration:**
 * - Created by PianoRollEditor
 * - Modified by GridControlPanel (via toggle buttons)
 * - Read by NoteComponent (during paint())
 *
 * **Future:**
 * This is a temporary solution for piano roll development. In the final project,
 * these settings should be integrated into an application-wide style system or
 * user preferences manager.
 *
 * @note All settings default to false (minimal visual clutter).
 *
 * @see GridControlPanel
 * @see NoteComponent
 * @see PianoRollEditor
 */
class GridStyleSheet {
public:
    //==============================================================================
    // Friend Classes

    /**
     * @brief Grants GridControlPanel write access to private style flags.
     *
     * This establishes a one-way relationship: GridControlPanel can modify settings
     * via direct member access, while other classes use public getters for read-only
     * access. This avoids cluttering the interface with setter methods.
     */
    friend class GridControlPanel;

    //==============================================================================
    // Lifecycle

    /**
     * @brief Constructs a style sheet with default settings (all false).
     *
     * Initializes all display options to disabled for minimal visual clutter.
     */
    GridStyleSheet();

    //==============================================================================
    // Display Option Accessors (Read-Only)

    /**
     * @brief Checks if MIDI note numbers should be drawn on notes.
     *
     * When true, NoteComponent should render the MIDI note number (0-127) on each
     * note rectangle during paint().
     *
     * @return true if MIDI numbers should be displayed, false otherwise
     */
    bool getDrawMIDINum();

    /**
     * @brief Checks if MIDI note name strings should be drawn on notes.
     *
     * When true, NoteComponent should render the note name string (e.g., "C4", "A#3")
     * on each note rectangle during paint(). Uses PConstants::pitches_names for
     * pitch class names.
     *
     * @return true if note names should be displayed, false otherwise
     * @see PConstants::pitches_names
     */
    bool getDrawMIDINoteStr();

    /**
     * @brief Checks if velocity values should be drawn on notes.
     *
     * When true, NoteComponent should render the MIDI velocity (0-127) on each
     * note rectangle during paint().
     *
     * @return true if velocity values should be displayed, false otherwise
     */
    bool getDrawVelocity();

private:
    //==============================================================================
    // Private Members (writable by GridControlPanel via friend access)

    bool drawMIDINum;      ///< Flag: draw MIDI note numbers on notes
    bool drawMIDINoteStr;  ///< Flag: draw note name strings on notes (e.g., "C4")
    bool drawVelocity;     ///< Flag: draw velocity values on notes
};


#endif //GRIDSTYLESHEET_H
