//
// Created by Joseph Rockwell on 4/11/25.
//

#ifndef PCONSTANTS_H
#define PCONSTANTS_H

//==============================================================================
// Type Definitions

#ifndef u8
/**
 * @brief Unsigned 8-bit integer type alias.
 *
 * Convenience typedef for unsigned char, commonly used for MIDI data values
 * (note numbers, velocities, etc.) which range 0-127.
 */
typedef unsigned char u8;
#endif

#ifndef st_int
/**
 * @brief Standard unsigned integer type alias.
 *
 * Convenience typedef for unsigned int, used for general-purpose counters
 * and indices in the piano roll editor.
 */
typedef unsigned int st_int;
#endif

//==============================================================================
/**
 * @brief Piano Roll Editor constants and utilities namespace.
 *
 * The PRE (Piano Roll Editor) namespace contains shared constants used throughout
 * the piano roll editor components, including MIDI timing resolution and pitch
 * name string mappings.
 *
 * **Key Constants:**
 * - defaultResolution: MIDI ticks per quarter note (480 PPQN)
 * - pitches_names: Chromatic pitch class names for display
 *
 * **Integration:**
 * Referenced by NoteGridComponent, TimelineComponent, KeyboardComponent, and
 * other piano roll components for consistent timing and pitch representation.
 *
 * @see NoteGridComponent
 * @see KeyboardComponent
 * @see GridStyleSheet
 */
namespace PRE {
    //==============================================================================
    // MIDI Timing Resolution

    /**
     * @brief MIDI resolution in ticks per quarter note (PPQN).
     *
     * Defines the timing granularity for MIDI note positions and lengths. A value
     * of 480 PPQN is a standard MIDI file format resolution that provides fine
     * timing precision:
     *
     * - 480 ticks = 1 quarter note (1 beat)
     * - 240 ticks = 1 eighth note (1/2 beat)
     * - 120 ticks = 1 sixteenth note (1/4 beat)
     * - 60 ticks = 1 thirty-second note (1/8 beat)
     * - 30 ticks = 1 sixty-fourth note (1/16 beat, hemidemisemiquaver)
     *
     * This resolution allows precise quantization down to 1/64th notes while
     * remaining compatible with standard MIDI file formats.
     *
     * @note Although this could be changed, 480 PPQN is a well-established standard
     *       that balances precision with computational efficiency.
     *
     * @see GridControlPanel::fractionsOfBeat
     * @see NoteGridComponent
     */
    static const int defaultResolution = 480; // per quarter note (PPQN)

    //==============================================================================
    // Pitch Name Mappings

    /**
     * @brief Chromatic pitch class names for MIDI notes.
     *
     * Array of 12 pitch class names (C through B) for displaying note names in
     * the piano roll editor. Index corresponds to pitch class modulo 12:
     *
     * - Index 0: C
     * - Index 1: C# (sharp, not Db flat notation)
     * - Index 2: D
     * - ...
     * - Index 11: B
     *
     * **Usage:**
     * To get the pitch name for a MIDI note number (0-127):
     * ```cpp
     * int midiNote = 60; // Middle C (C4)
     * int pitchClass = midiNote % 12; // 0
     * const char* noteName = PRE::pitches_names[pitchClass]; // "C"
     * ```
     *
     * **Enharmonic Spelling:**
     * This array uses sharp notation for accidentals (C#, D#, F#, G#, A#) rather
     * than flat notation (Db, Eb, Gb, Ab, Bb). This is a common convention in
     * MIDI-based applications.
     *
     * @see KeyboardComponent
     * @see GridStyleSheet::getDrawMIDINoteStr
     * @see NoteComponent
     */
    static const char *pitches_names[] = {
        "C",   ///< Pitch class 0
        "C#",  ///< Pitch class 1 (C sharp)
        "D",   ///< Pitch class 2
        "D#",  ///< Pitch class 3 (D sharp)
        "E",   ///< Pitch class 4
        "F",   ///< Pitch class 5
        "F#",  ///< Pitch class 6 (F sharp)
        "G",   ///< Pitch class 7
        "G#",  ///< Pitch class 8 (G sharp)
        "A",   ///< Pitch class 9
        "A#",  ///< Pitch class 10 (A sharp)
        "B",   ///< Pitch class 11
    };
}

#endif //PCONSTANTS_H
