//
// Created by Joseph Rockwell on 4/11/25.
//

#ifndef GRIDCONTROLPANEL_H
#define GRIDCONTROLPANEL_H

#include "GridStyleSheet.h"
#include "NoteGridComponent.h"
#include "PConstants.h"

//==============================================================================
/**
 * @brief Control panel for piano roll grid configuration and note operations.
 *
 * GridControlPanel provides UI controls for adjusting the piano roll editor's
 * quantization, zoom, and editing behavior. It sits at the top of the piano roll
 * and allows users to change grid resolution, delete selected notes, and configure
 * display options.
 *
 * **Features:**
 * - Quantization selector (1/64 to whole note)
 * - Delete selected notes button
 * - Grid zoom configuration callback
 * - Future: Toggle buttons for MIDI note display options
 *
 * **Quantization Levels:**
 * The panel supports 5 quantization levels, mapped to quarter-note fractions:
 * - Quantise64: 1/64th note (1/16th of a beat)
 * - Quantise32: 1/32nd note (1/8th of a beat)
 * - Quantise16: 1/16th note (1/4 of a beat)
 * - Quantise8: 1/8th note (1/2 beat)
 * - Quantise4: 1/4 note (whole beat)
 *
 * **Integration:**
 * Used by PianoRollEditor as a top toolbar. Communicates with NoteGridComponent
 * via direct reference for note deletion and quantization updates. Grid zoom
 * changes are propagated via configureGrid callback.
 *
 * @note Tracktion Engine uses quarter-notes as the base beat unit, so all
 *       quantization values are expressed as fractions of a beat.
 *
 * @see PianoRollEditor
 * @see NoteGridComponent
 * @see GridStyleSheet
 * @see PRE::defaultResolution
 */
class GridControlPanel : public juce::Component {
public:
    //==============================================================================
    // Nested Types

    /**
     * @brief Enumeration of quantization preset keys for the ComboBox.
     *
     * Maps to fractionsOfBeat lookup table for conversion to beat fractions.
     */
    enum quantisationKeys {
        Quantise64 = 1,  ///< 1/64th note (finest resolution)
        Quantise32,      ///< 1/32nd note
        Quantise16,      ///< 1/16th note
        Quantise8,       ///< 1/8th note
        Quantise4,       ///< 1/4 note (whole beat, coarsest resolution)
    };

    //==============================================================================
    // Lifecycle

    /**
     * @brief Constructs the control panel with grid and style references.
     *
     * Initializes UI controls (quantization ComboBox, delete button) and sets up
     * listeners. The noteGrid and styleSheet are stored by reference and must
     * remain valid for the panel's lifetime.
     *
     * @param component Reference to the NoteGridComponent (for note operations)
     * @param styleSheet Reference to the shared GridStyleSheet (for display config)
     */
    GridControlPanel (NoteGridComponent & component, GridStyleSheet & styleSheet);

    /**
     * @brief Destructor (cleans up UI controls).
     */
    ~GridControlPanel () override;

    //==============================================================================
    // Component Overrides

    /**
     * @brief Lays out child UI controls (ComboBox, buttons).
     *
     * Positions controls in a horizontal toolbar layout with appropriate spacing.
     */
    void resized () override;

    /**
     * @brief Renders the panel background.
     *
     * Draws a simple background fill for the control panel.
     *
     * @param g Graphics context for rendering
     */
    void paint (juce::Graphics & g) override;

    //==============================================================================
    // Callback Functions (set by PianoRollEditor)

    /**
     * @brief Callback triggered when grid zoom configuration changes.
     *
     * Invoked when the user adjusts zoom/spacing controls. Parent component
     * (PianoRollEditor) should update TimelineComponent and NoteGridComponent
     * accordingly.
     *
     * @param pixelsPerBar New horizontal zoom factor (pixels per bar)
     * @param noteHeight New vertical spacing (pixels per MIDI note)
     */
    std::function<void(int pixelsPerBar, int noteHeight)> configureGrid;

private:
    //==============================================================================
    // Private Members

    /**
     * @brief Quantization lookup table: maps enum keys to beat fractions.
     *
     * Since Tracktion Engine uses quarter-notes as beats, this table converts
     * musical note values to fractional beat positions. For example:
     * - Quantise16 (1/16th note) = 0.25 beats (1/4 of a quarter note)
     * - Quantise4 (1/4 note) = 1.0 beat (one quarter note)
     */
    const std::map<int, float> fractionsOfBeat = {
        {Quantise64, 1.f / 16.f},  ///< 1/64th note = 1/16th beat
        {Quantise32, 1.f / 8.f},   ///< 1/32nd note = 1/8th beat
        {Quantise16, 1.f / 4.f},   ///< 1/16th note = 1/4 beat
        {Quantise8, 1.f / 2.f},    ///< 1/8th note = 1/2 beat
        {Quantise4, 1.f},          ///< 1/4 note = 1 beat
    };

    NoteGridComponent & noteGrid;  ///< Reference to the note grid (for delete operations)
    GridStyleSheet & styleSheet;   ///< Reference to shared style configuration

    juce::TextButton deleteNotes;  ///< Button to delete selected notes

    // Future display toggles (currently unused):
    // juce::ToggleButton drawMIDINotes, drawMIDIText, drawVelocity;

    juce::ComboBox quantisationValue;  ///< Dropdown selector for quantization level
};


#endif //GRIDCONTROLPANEL_H
