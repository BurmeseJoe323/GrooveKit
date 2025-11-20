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

namespace te = tracktion::engine;

/**
 * @brief Custom viewport that synchronizes scrolling across multiple piano roll viewports.
 *
 * CustomViewport extends juce::Viewport to enable synchronized scrolling behavior:
 * - Vertical scrolling: Synchronizes KeyboardComponent and NoteGridComponent
 * - Horizontal scrolling: Synchronizes TimelineComponent and NoteGridComponent
 *
 * This ensures the piano keyboard, note grid, and timeline ruler stay visually aligned
 * as the user scrolls through the piano roll editor.
 *
 * **Usage**: Attach a callback to `positionMoved` to propagate scroll changes to other viewports.
 */
class CustomViewport : public juce::Viewport
{
public:
    /**
     * @brief Called when the visible area of the viewport changes (scroll or resize).
     *
     * Invokes the `positionMoved` callback with the new scroll position, allowing
     * other viewports to synchronize their positions.
     *
     * @param newVisibleArea The new visible rectangle within the viewport.
     */
    void visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea) override
    {
        Viewport::visibleAreaChanged (newVisibleArea);
        if (positionMoved)
        {
            positionMoved (getViewPositionX(), getViewPositionY());
        }
    }

    std::function<void (int, int)> positionMoved; ///< Callback invoked when viewport position changes (x, y).
};

/**
 * @brief Main piano roll editor component for MIDI clip editing.
 *
 * PianoRollEditor provides a complete MIDI editor interface with:
 * - **KeyboardComponent**: Piano keyboard visualization (left side, vertical)
 * - **NoteGridComponent**: Interactive note grid for editing MIDI notes
 * - **TimelineComponent**: Beat/bar ruler (top, horizontal)
 * - **GridControlPanel**: Visual style controls (optional, for development)
 *
 * **Coordinate System**:
 * - Horizontal: Bars and beats (configurable `pixelsPerBar`)
 * - Vertical: MIDI notes 0-127 (configurable `noteHeight`)
 *
 * **Features**:
 * - Add, move, resize, delete MIDI notes via mouse interaction
 * - Synchronized scrolling across keyboard, grid, and timeline
 * - Playback position marker overlay
 * - Customizable visual style via GridStyleSheet
 * - Drum track awareness (highlights MIDI notes 36-51 in blue)
 *
 * **Usage**:
 * 1. Construct with AppEngine and MidiClip or track index
 * 2. Call `setup()` to configure grid dimensions
 * 3. Optionally register callbacks: `onClose`, `onEdit`, `sendChange`
 * 4. Call `setClip()` to switch between clips dynamically
 *
 * **Thread Safety**: Must be used from message thread (JUCE UI component).
 *
 * @see NoteGridComponent for note editing implementation
 * @see KeyboardComponent for piano keyboard visualization
 * @see TimelineComponent for timeline ruler
 */
class PianoRollEditor : public juce::Component
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs a PianoRollEditor for a specific MIDI clip.
     *
     * @param engine Reference to AppEngine (provides Edit, MidiEngine, etc.).
     * @param clip Pointer to the MidiClip to edit (must not be null).
     */
    PianoRollEditor (AppEngine& engine, te::MidiClip* clip);

    /**
     * @brief Constructs a PianoRollEditor for the first clip on a track.
     *
     * Automatically retrieves the first MIDI clip on the specified track.
     * If no clip exists, creates a default 8-beat clip.
     *
     * @param engine Reference to AppEngine.
     * @param trackIndex 0-based index of the track to edit.
     */
    PianoRollEditor (AppEngine& engine, int trackIndex);

    /** Destructor. */
    ~PianoRollEditor() override = default;

    //==============================================================================
    // Setup and Configuration

    /**
     * @brief Initializes the piano roll grid dimensions and zoom level.
     *
     * Must be called after construction before displaying the editor.
     * Configures the size of the grid based on number of bars and zoom parameters.
     *
     * @param bars Number of bars (measures) to display in the editor.
     * @param pixelsPerBar Horizontal zoom level (pixels per bar). Higher = more zoomed in.
     * @param noteHeight Vertical zoom level (pixels per MIDI note). Higher = taller notes.
     */
    void setup (const int bars, const int pixelsPerBar, const int noteHeight);

    /**
     * @brief Updates the number of bars displayed in the editor.
     *
     * Dynamically resizes the grid to accommodate more or fewer bars.
     * Useful for extending the editor as the clip grows.
     *
     * @param newNumberOfBars New number of bars to display.
     */
    void updateBars (const int newNumberOfBars);

    /**
     * @brief Changes the currently edited MIDI clip.
     *
     * Allows switching between clips without destroying and recreating the editor.
     * Updates the note grid, keyboard highlighting, and timeline to reflect the new clip.
     *
     * @param clip Pointer to the new MidiClip to edit (must not be null).
     */
    void setClip (te::MidiClip* clip);

    /**
     * @brief Sets the visual style of the grid (colors, fonts, etc.).
     *
     * @param styleSheet The GridStyleSheet to apply.
     */
    void setStyleSheet (GridStyleSheet styleSheet);

    //==============================================================================
    // Scrolling and View Control

    /**
     * @brief Programmatically sets the scroll position of all viewports.
     *
     * Synchronizes the scroll position across keyboard, grid, and timeline viewports.
     *
     * @param x Horizontal scroll position in pixels.
     * @param y Vertical scroll position in pixels.
     */
    void setScroll (double x, double y);

    //==============================================================================
    // Playback Marker

    /**
     * @brief Updates the playback position marker overlay.
     *
     * Called during playback to render a vertical line showing current playback position.
     *
     * @param ticks Current playback position in Tracktion ticks.
     * @param isVisible Whether the marker should be displayed.
     */
    void setPlaybackMarkerPosition (const st_int ticks, bool isVisible = true);

    //==============================================================================
    // Data Access

    /**
     * @brief Returns the MIDI note sequence of the current clip.
     *
     * Provides access to the underlying MidiList for reading note data.
     *
     * @return Reference to the te::MidiList containing MIDI notes.
     */
    const te::MidiList& getSequence();

    /**
     * @brief Returns the control panel for adjusting visual style.
     *
     * **Note**: Control panel is intended for development/experimentation.
     * Consider removing or repurposing before production release.
     *
     * @return Reference to the GridControlPanel.
     */
    GridControlPanel& getControlPanel();

    //==============================================================================
    // UI Control

    /**
     * @brief Shows or hides the grid control panel.
     *
     * @param state True to show, false to hide.
     */
    void showControlPanel (bool state);

    //==============================================================================
    // Component Overrides

    /** @brief Renders the background of the piano roll editor. */
    void paint (juce::Graphics& g) override;

    /** @brief Renders the playback position marker on top of child components. */
    void paintOverChildren (juce::Graphics& g) override;

    /** @brief Handles layout of child components (keyboard, grid, timeline, control panel). */
    void resized() override;

    /**
     * @brief Handles keyboard shortcuts for the piano roll editor.
     *
     * Supported shortcuts:
     * - Escape: Close the editor (calls `onClose` callback)
     *
     * @param key The key press event.
     * @return True if the key was handled.
     */
    bool keyPressed (const juce::KeyPress& key) override;

    //==============================================================================
    // Callbacks

    std::function<void()> onClose;                        ///< Called when the editor is closed (e.g., Escape key or close button).
    std::function<void()> onEdit;                         ///< Called when the MIDI clip is edited (note added, moved, resized, or deleted).
    std::function<void (int note, int velocity)> sendChange; ///< Called when a note is interacted with. Use for live MIDI preview during editing.

private:
    //==============================================================================
    // Grid Parameters

    float pixelsPerBar = 550;  ///< Horizontal zoom level (pixels per bar).
    float noteHeight = 15;     ///< Vertical zoom level (pixels per MIDI note).
    int numBars = 10;          ///< Number of bars displayed in the editor.

    GridStyleSheet gridStyleSheet; ///< Visual style for the grid (colors, fonts, etc.).

    //==============================================================================
    // Child Components

    KeyboardComponent keyboard;   ///< Piano keyboard visualizer (left side).
    NoteGridComponent noteGrid;   ///< Interactive MIDI note grid (center).
    TimelineComponent timeline;   ///< Beat/bar ruler (top).

    CustomViewport keyboardView;  ///< Viewport for keyboard component.
    CustomViewport gridView;      ///< Viewport for note grid component.
    CustomViewport timelineView;  ///< Viewport for timeline component.

    GridControlPanel controlPanel; ///< Optional control panel for adjusting grid visuals (development tool).

    //==============================================================================
    // Playback State

    st_int playbackTicks;        ///< Current playback position in Tracktion ticks.
    bool showPlaybackMarker;     ///< Whether to display the playback position marker.

    //==============================================================================
    // UI Controls

    juce::TextButton closeButton { "x" }; ///< Button to close the piano roll editor.
};
#endif
