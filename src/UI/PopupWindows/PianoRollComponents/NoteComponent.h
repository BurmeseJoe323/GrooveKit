//
// Created by Joseph Rockwell on 4/13/25.
//

#ifndef NOTECOMPONENT_H
#define NOTECOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "PConstants.h"
# include "GridStyleSheet.h"

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

//==============================================================================
/**
 * @brief Visual representation and editor for a single MIDI note in the piano roll.
 *
 * NoteComponent is the interactive UI element representing a MIDI note (te::MidiNote)
 * in the piano roll grid. It provides drag-to-move, edge-resize, and multi-selection
 * capabilities for intuitive MIDI editing.
 *
 * **Features:**
 * - Visual note representation with customizable colors
 * - Drag to move notes horizontally (time) and vertically (pitch)
 * - Right-edge resize to adjust note length/duration
 * - Selection state with visual feedback
 * - Multi-note editing support (parallel drag/resize operations)
 * - Mouse hover effects for interactive feedback
 * - Binds to Tracktion Engine MidiNote model for persistence
 *
 * **Mouse Interaction:**
 * - Click: Select note (triggers onNoteSelect)
 * - Drag (body): Move note position (triggers onDragging, onPositionMoved)
 * - Drag (right edge): Resize note length (triggers onEdgeDragging, onLengthChange)
 * - Hover: Visual highlight feedback
 *
 * **Multi-Selection Behavior:**
 * When multiple notes are selected, drag/resize operations are broadcast to all
 * selected NoteComponents via callbacks. The isMultiDrag flag coordinates simultaneous
 * edits while preserving relative positioning/timing.
 *
 * **Coordinate System:**
 * - X-axis: Time (pixels map to beats via NoteGridComponent)
 * - Y-axis: Pitch (pixels map to MIDI note numbers 0-127)
 * - Width: Note duration (in pixel-based time units)
 *
 * **Threading:**
 * All operations must occur on the JUCE message thread. Model updates propagate
 * to Tracktion Engine synchronously.
 *
 * @see NoteGridComponent
 * @see GridStyleSheet
 * @see te::MidiNote
 * @see PianoRollEditor
 */
class NoteComponent : public juce::Component, public juce::ComponentDragger {
public:
    //==============================================================================
    // Nested Types

    /**
     * @brief Selection state enumeration for visual feedback.
     */
    enum eState {
        eNone,      ///< Note is not selected (default state)
        eSelected,  ///< Note is selected (part of multi-selection group)
    };

    /**
     * @brief Deprecated multi-edit state structure (unused, kept for compatibility).
     */
    struct MultiEditState {
        int startWidth;                     ///< Used when resizing the noteComponents' length
        bool coordinatesDiffer;             ///< Flag indicating external coordinate changes
        juce::Rectangle<int> startingBounds; ///< Original bounds before multi-edit operation
    };

    //==============================================================================
    // Lifecycle

    /**
     * @brief Constructs a NoteComponent with style configuration.
     *
     * Initializes the note editor with default state (unselected, no model bound).
     * The styleSheet is stored by reference and must remain valid for the component's lifetime.
     *
     * @param styleSheet Reference to shared grid style configuration
     */
    NoteComponent(GridStyleSheet styleSheet);

    /** @brief Default destructor. */
    ~NoteComponent() override = default;

    //==============================================================================
    // Component Overrides

    /**
     * @brief Renders the note rectangle with state-based styling.
     *
     * Draws the note with different colors/borders based on:
     * - Selection state (highlighted border)
     * - Mouse hover state (lighter fill)
     * - Custom color (if set)
     *
     * @param g Graphics context for rendering
     */
    void paint (juce::Graphics & g) override;

    /**
     * @brief Updates edge resizer bounds when component is resized.
     *
     * Positions the right-edge ResizableEdgeComponent for note length editing.
     */
    void resized () override;

    //==============================================================================
    // Appearance Configuration

    /**
     * @brief Sets a custom color for this note (overrides default styling).
     *
     * Useful for color-coding notes by velocity, channel, or user preference.
     *
     * @param c Custom color to apply
     */
    void setCustomColour (juce::Colour c);

    //==============================================================================
    // Model Binding

    /**
     * @brief Binds this component to a Tracktion MidiNote model.
     *
     * Establishes the data binding between UI and engine. All drag/resize operations
     * will update the bound MidiNote's start time, pitch, and length.
     *
     * @param model Pointer to the MidiNote this component represents (ownership not transferred)
     */
    void setModel(te::MidiNote *model);

    /**
     * @brief Retrieves the bound MidiNote model.
     *
     * @return Pointer to the Tracktion MidiNote, or nullptr if not bound
     */
    te::MidiNote *getModel();

    //==============================================================================
    // Selection State Management

    /**
     * @brief Sets the selection state of this note.
     *
     * Updates visual styling to reflect selection. Selected notes participate
     * in multi-note drag/resize operations.
     *
     * @param state New selection state (eNone or eSelected)
     */
    void setState (eState state);

    /**
     * @brief Gets the current selection state.
     *
     * @return Current eState value
     */
    eState getState ();

    //==============================================================================
    // Mouse Event Handlers

    /**
     * @brief Handles mouse enter events (hover start).
     *
     * Updates visual state to highlight the note under the cursor.
     *
     * @param event Mouse event data (unused)
     */
    void mouseEnter (const juce::MouseEvent&) override;

    /**
     * @brief Handles mouse exit events (hover end).
     *
     * Removes hover highlight styling.
     *
     * @param event Mouse event data (unused)
     */
    void mouseExit  (const juce::MouseEvent&) override;

    /**
     * @brief Handles mouse down events (click/drag start).
     *
     * - Left click: Select note (triggers onNoteSelect callback)
     * - Stores initial drag position for delta calculations
     * - Begins JUCE ComponentDragger operation
     *
     * @param event Mouse event with click position and modifiers
     */
    void mouseDown  (const juce::MouseEvent&) override;

    /**
     * @brief Handles mouse up events (click/drag end).
     *
     * - Finalizes drag operation
     * - Commits coordinate changes to model (if coordinatesDiffer flag set)
     * - Triggers onPositionMoved or onLengthChange callbacks
     *
     * @param event Mouse event data
     */
    void mouseUp    (const juce::MouseEvent&) override;

    /**
     * @brief Handles mouse drag events (note move/resize in progress).
     *
     * - Body drag: Moves note horizontally/vertically (triggers onDragging)
     * - Edge drag: Resizes note length (triggers onEdgeDragging)
     * - For multi-selection, broadcasts to all selected notes
     *
     * @param event Mouse event with current drag position
     */
    void mouseDrag  (const juce::MouseEvent&) override;

    /**
     * @brief Handles mouse move events (cursor changes based on hover region).
     *
     * Updates cursor to resize icon when hovering over right edge.
     *
     * @param event Mouse event with current position
     */
    void mouseMove  (const juce::MouseEvent&) override;

    //==============================================================================
    // Callback Functions (set by NoteGridComponent)

    std::function<void(NoteComponent *, const juce::MouseEvent &)> onNoteSelect;   ///< Triggered on note click (for selection management)
    std::function<void(NoteComponent *)> onPositionMoved;                          ///< Triggered after note position change (time/pitch)
    std::function<void(NoteComponent *, const juce::MouseEvent &)> onDragging;     ///< Triggered during note drag (for multi-note parallel moves)
    std::function<void(NoteComponent *, const juce::MouseEvent &)> onEdgeDragging; ///< Triggered during edge resize (for multi-note parallel resizes)
    std::function<void(NoteComponent *)> onLengthChange;                           ///< Triggered after note length change

    //==============================================================================
    // Public State Variables (accessed by NoteGridComponent)

    int minWidth = 10;              ///< Minimum note width in pixels (prevents zero-length notes)
    int startWidth;                 ///< Width at drag start (for delta calculations during resize)
    int startX, startY;             ///< Position at drag start (for delta calculations during move)
    bool coordinatesDiffer;         ///< Flag: true when UI position differs from model (deferred sync)
    bool isMultiDrag;               ///< Flag: true when participating in multi-note drag operation

private:
    //==============================================================================
    // Private Members

    GridStyleSheet styleSheet;               ///< Copy of style configuration (safe ownership)
    juce::ResizableEdgeComponent edgeResizer; ///< Right-edge resize handle component

    bool mouseOver;        ///< True when mouse is hovering over this note
    bool useCustomColour;  ///< True when custom color override is active
    bool resizeEnabled;    ///< True when edge resizing is enabled (currently unused)
    bool velocityEnabled;  ///< True when velocity editing is enabled (currently unused)
    int startVelocity;     ///< Velocity at drag start (for future velocity editing feature)

    juce::Colour customColour;  ///< Custom color override (if useCustomColour is true)

    te::MidiNote *model;        ///< Pointer to bound Tracktion MidiNote (not owned)
    juce::MouseCursor normal;   ///< Standard mouse cursor (cached for restoration)
    eState state;               ///< Current selection state
};

#endif //NOTECOMPONENT_H
