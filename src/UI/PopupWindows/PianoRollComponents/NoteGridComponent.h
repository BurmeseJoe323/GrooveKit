//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef NOTEGRIDCOMPONENT_H
#define NOTEGRIDCOMPONENT_H

#include "../../../AppEngine/AppEngine.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include <memory>

#include "GridStyleSheet.h"
#include "NoteComponent.h"
#include "PConstants.h"

namespace te = tracktion::engine;

/**
 * @brief Visual selection box displayed during mouse drag selection.
 *
 * SelectionBox is a semi-transparent overlay component that appears when the user
 * drags to select multiple notes in the piano roll grid. It renders a white box
 * with 50% alpha to indicate the selection area.
 *
 * The box dimensions are controlled by the parent NoteGridComponent during mouse drag events.
 */
class SelectionBox : public juce::Component
{
public:
    /**
     * @brief Renders the selection box as a semi-transparent white rectangle.
     *
     * @param g Graphics context for rendering.
     */
    void paint (juce::Graphics& g) override
    {
        juce::Colour c = juce::Colours::white;
        c = c.withAlpha ((float) 0.5);
        g.fillAll (c);
    }

    int startX; ///< Starting X coordinate of the selection drag.
    int startY; ///< Starting Y coordinate of the selection drag.
};

/**
 * @brief Interactive grid component for editing MIDI notes in the piano roll.
 *
 * NoteGridComponent is the core editing surface of the piano roll editor. It provides:
 * - **Visual grid**: Bar/beat grid lines with alternating row colors for black/white piano keys
 * - **Note editing**: Add, move, resize, delete MIDI notes via mouse interaction
 * - **Selection**: Click or drag-select notes for batch operations
 * - **Quantization**: Snap notes to grid divisions (e.g., 1/4, 1/8, 1/16)
 * - **Drum track awareness**: Highlights MIDI notes 36-51 (C1-D#2) in blue for drum tracks
 *
 * **Coordinate System**:
 * - **Horizontal**: Bars and beats (configurable `pixelsPerBar`)
 * - **Vertical**: MIDI note numbers 0-127 (configurable `noteCompHeight`)
 *
 * **Mouse Interactions**:
 * - **Click empty space**: Add new note at clicked position (quantized)
 * - **Click note**: Select note (Cmd/Ctrl to multi-select)
 * - **Drag note**: Move selected notes (quantized to grid)
 * - **Drag note edge**: Resize note length (quantized to grid)
 * - **Drag empty space**: Draw selection box to select multiple notes
 * - **Double-click note**: Delete note
 * - **Delete/Backspace key**: Delete all selected notes
 *
 * **Data Flow**:
 * - Reads MIDI notes from `te::MidiClip::getSequence()`
 * - Creates NoteComponent visuals for each MIDI note
 * - Edits propagate to MidiClip via `te::MidiList::addNote()`, `removeNote()`, etc.
 * - Calls `onEdit` callback after any modification
 *
 * **Thread Safety**: Must be used from message thread (JUCE UI component).
 *
 * @see NoteComponent for individual note visualization
 * @see PianoRollEditor for the complete piano roll interface
 */
class NoteGridComponent : public juce::Component, public juce::KeyListener
{
public:
    // Avoid hiding base overload when providing a 2-arg keyPressed below
    using juce::Component::keyPressed;
    //==============================================================================
    // Data Structures

    /**
     * @brief Defines a musical time signature (e.g., 4/4, 3/4, 6/8).
     */
    struct TimeSignature
    {
        unsigned int beatsPerBar; ///< Number of beats per measure (numerator).
        unsigned int beatValue;   ///< Note value that gets one beat (denominator, e.g., 4 = quarter note).
    };

    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the note grid component.
     *
     * @param sheet Reference to the visual style sheet for grid rendering.
     * @param engine Reference to AppEngine (provides Edit, MidiEngine, etc.).
     * @param clip Pointer to the MidiClip to edit (must not be null).
     */
    NoteGridComponent (GridStyleSheet& sheet, AppEngine& engine, te::MidiClip* clip);

    /**
     * @brief Destructor. Cleans up note components and key listener registration.
     */
    ~NoteGridComponent() override;

    //==============================================================================
    // Setup and Configuration

    /**
     * @brief Initializes the grid dimensions and zoom level.
     *
     * Configures the grid size based on number of bars and zoom parameters.
     * Creates NoteComponent visuals for all existing MIDI notes in the clip.
     *
     * @param pixelsPerBar Horizontal zoom level (pixels per bar). Higher = more zoomed in.
     * @param compHeight Vertical zoom level (pixels per MIDI note). Higher = taller notes.
     * @param bars Number of bars (measures) to display in the grid.
     */
    void setupGrid (float pixelsPerBar, float compHeight, const int bars);

    /**
     * @brief Changes the currently edited MIDI clip.
     *
     * Clears existing note components and creates new ones for the new clip's notes.
     *
     * @param newClip Pointer to the new MidiClip to edit (must not be null).
     */
    void setClip (te::MidiClip* newClip);

    /**
     * @brief Sets the time signature for grid line rendering and quantization.
     *
     * @param beatsPerBar Number of beats per measure (e.g., 4 for 4/4 time).
     * @param beatValue Note value that gets one beat (e.g., 4 for quarter note).
     */
    void setTimeSignature (unsigned int beatsPerBar, unsigned int beatValue);

    /**
     * @brief Returns whether this grid is editing a drum track.
     *
     * Used by PianoRollEditor to synchronize keyboard highlighting. (Written by Claude Code)
     *
     * @return True if editing a drum track
     */
    bool getIsDrumTrack() const { return isDrumTrack; }

    /**
     * @brief Gets the current time signature's beats per bar.
     *
     * @return Number of beats per measure.
     */
    unsigned int getBeatsPerBar() const { return timeSignature.beatsPerBar; }

    /**
     * @brief Sets the quantization grid resolution.
     *
     * Determines the smallest grid division for note placement and editing.
     * Examples: 1.0 = whole note, 0.25 = quarter note, 0.125 = eighth note.
     *
     * @param newVal Quantization value in beats (1.0 = whole note).
     */
    void setQuantisation (float newVal);

    //==============================================================================
    // Data Access

    /**
     * @brief Returns the MIDI note sequence for the current clip.
     *
     * Provides read/write access to the underlying MidiList for direct manipulation.
     *
     * @return Reference to te::MidiList containing MIDI notes.
     */
    te::MidiList& getSequence();

    /**
     * @brief Returns the height of each note row (pixels per MIDI note).
     *
     * @return Vertical zoom level in pixels.
     */
    float getNoteCompHeight() const;

    /**
     * @brief Returns the horizontal zoom level.
     *
     * @return Pixels per bar.
     */
    float getPixelsPerBar() const;

    /**
     * @brief Sets the active MIDI clip being edited.
     *
     * @param clip Pointer to MidiClip (not owned by this component).
     */
    void setActiveClip (te::MidiClip* clip);

    /**
     * @brief Returns the currently edited clip.
     *
     * @return Pointer to MidiClip (not owned).
     */
    te::MidiClip* getActiveClip();

    /**
     * @brief Returns the currently edited clip (const version).
     *
     * @return Const pointer to MidiClip (not owned).
     */
    const te::MidiClip* getActiveClip() const;

    /**
     * @brief Returns the underlying MidiNote models for all selected notes.
     *
     * Used for batch operations on selected notes.
     *
     * @return Array of pointers to selected te::MidiNote objects.
     */
    juce::Array<te::MidiNote*> getSelectedModels();

    //==============================================================================
    // Note Management

    /**
     * @brief Deletes all currently selected notes.
     *
     * Removes selected notes from the clip's MIDI sequence and destroys their
     * corresponding NoteComponent visuals. Calls `onEdit` callback.
     */
    void deleteAllSelected();

    /**
     * @brief Updates the visual positions of all note components.
     *
     * Called after zoom changes or clip changes to refresh note positions.
     */
    void setPositions();

    //==============================================================================
    // NoteComponent Event Handlers (called by child NoteComponent instances)

    /**
     * @brief Called when a note component is clicked for selection.
     *
     * @param noteComp The clicked NoteComponent.
     * @param e The mouse event (Cmd/Ctrl modifier for multi-select).
     */
    void noteCompSelected (NoteComponent* noteComp, const juce::MouseEvent& e);

    /**
     * @brief Called when a note component is moved to a new position.
     *
     * Updates the underlying MidiNote model and repositions the visual component.
     *
     * @param noteComp The moved NoteComponent.
     * @param callResize Whether to trigger a resize event (default: true).
     */
    void noteCompPositionMoved (NoteComponent* noteComp, bool callResize = true);

    /**
     * @brief Called when a note component's length is changed.
     *
     * Updates the underlying MidiNote model's length.
     *
     * @param noteComp The resized NoteComponent.
     */
    void noteCompLengthChanged (NoteComponent* noteComp);

    /**
     * @brief Called during note dragging (moving the entire note).
     *
     * @param noteComp The dragged NoteComponent.
     * @param e The mouse event containing drag position.
     */
    void noteCompDragging (NoteComponent* noteComp, const juce::MouseEvent& e);

    /**
     * @brief Called during note edge dragging (resizing the note).
     *
     * @param noteComp The resized NoteComponent.
     * @param e The mouse event containing drag position.
     */
    void noteEdgeDragging (NoteComponent* noteComp, const juce::MouseEvent& e);

    //==============================================================================
    // Component Overrides

    /**
     * @brief Renders the grid background, bar lines, and beat lines.
     *
     * Draws alternating row colors for black/white piano keys and highlights drum
     * sampler note range (MIDI 36-51) in blue for drum tracks.
     *
     * @param g Graphics context for rendering.
     */
    void paint (juce::Graphics& g) override;

    /**
     * @brief Handles layout of child note components.
     */
    void resized() override;

    //==============================================================================
    // Mouse Event Handling

    /**
     * @brief Handles mouse down events for adding notes or starting selection.
     *
     * - Click empty space: Add new note at clicked position (quantized)
     * - Click and drag: Start selection box
     *
     * @param e Mouse event.
     */
    void mouseDown (const juce::MouseEvent& e) override;

    /**
     * @brief Handles mouse drag events for selection box.
     *
     * @param e Mouse event containing drag position.
     */
    void mouseDrag (const juce::MouseEvent& e) override;

    /**
     * @brief Handles mouse up events to finalize selection.
     *
     * Selects all notes within the selection box area.
     *
     * @param e Mouse event.
     */
    void mouseUp (const juce::MouseEvent& e) override;

    /**
     * @brief Handles double-click events to delete notes.
     *
     * Double-clicking a note deletes it immediately.
     *
     * @param e Mouse event.
     */
    void mouseDoubleClick (const juce::MouseEvent& e) override;

    //==============================================================================
    // Keyboard Event Handling

    /**
     * @brief Handles keyboard shortcuts for the note grid.
     *
     * Supported shortcuts:
     * - Delete/Backspace: Delete all selected notes
     *
     * @param key The key press event.
     * @param originatingComponent The component that originated the key press (unused).
     * @return True if the key was handled.
     */
    bool keyPressed (const juce::KeyPress& key, Component* originatingComponent) override;

    //==============================================================================
    // Callbacks

    std::function<void (int note, int velocity)> sendChange; ///< Called when a note is interacted with. Use for live MIDI preview.
    std::function<void()> onEdit;                            ///< Called when the clip is edited (note added, moved, resized, or deleted).

private:
    //==============================================================================
    // Internal Methods

    /**
     * @brief Invokes the onEdit callback if registered.
     */
    void sendEdit();

    /**
     * @brief Creates a new NoteComponent visual for a MidiNote model.
     *
     * @param midiNote Pointer to the MidiNote model.
     * @return Pointer to the newly created NoteComponent.
     */
    NoteComponent* addNewNoteComponent(te::MidiNote* midiNote);

    /**
     * @brief Resolves the current clip (const version).
     *
     * @return Const pointer to the current MidiClip.
     */
    const te::MidiClip* resolveClip() const;

    //==============================================================================
    // Coordinate Conversion Utilities

    /**
     * @brief Converts beat position to pixel X coordinate.
     *
     * @param beats Beat position.
     * @return Pixel X coordinate.
     */
    float beatsToX (float beats);

    /**
     * @brief Converts MIDI pitch to pixel Y coordinate.
     *
     * @param pitch MIDI note number (0-127).
     * @return Pixel Y coordinate.
     */
    float pitchToY (float pitch);

    /**
     * @brief Converts pixel X coordinate to beat position.
     *
     * @param x Pixel X coordinate.
     * @return Beat position.
     */
    float xToBeats (float x);

    /**
     * @brief Converts pixel Y coordinate to MIDI pitch.
     *
     * @param y Pixel Y coordinate.
     * @return MIDI note number (0-127).
     */
    int yToPitch (float y);

    //==============================================================================
    // Member Variables

    AppEngine& appEngine;             ///< Reference to AppEngine (not owned).
    te::MidiClip* clip = nullptr;     ///< Pointer to the currently edited clip (not owned).
    te::MidiClip* clipModel = nullptr; ///< Internal clip reference (not owned).

    GridStyleSheet& styleSheet;       ///< Visual style for grid rendering (not owned).
    SelectionBox selectorBox;         ///< Selection box component for drag selection.
    std::vector<std::unique_ptr<NoteComponent>> noteComps; ///< Visual components for each MIDI note.

    std::set<int> blackPitches = { 1, 3, 6, 8, 10 }; ///< MIDI note offsets for black piano keys (within octave).
    bool isDrumTrack = false;         ///< True if editing a drum track (for note highlighting). (Written by Claude Code)

    float noteCompHeight;             ///< Vertical zoom level (pixels per MIDI note).
    float pixelsPerBar;               ///< Horizontal zoom level (pixels per bar).
    TimeSignature timeSignature;      ///< Current time signature (e.g., 4/4).
    st_int ticksPerTimeSignature;     ///< Tracktion ticks per time signature unit.
    float currentQValue;              ///< Current quantization grid resolution (in beats).

    bool firstDrag;                   ///< Flag for first drag event (internal state).
    bool firstCall;                   ///< Flag for first call (internal state).
    int lastTrigger;                  ///< Last triggered note number (for MIDI preview).
};

#endif //NOTEGRIDCOMPONENT_H
