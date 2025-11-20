#pragma once

#include "../../AppEngine/AppEngine.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;
namespace t = tracktion;

/**
 * @brief Visual playhead overlay component showing the current transport position.
 *
 * PlayheadComponent renders a vertical line that moves across the timeline to indicate
 * the current playback or recording position. It is an overlay component that sits on
 * top of the TrackListComponent.
 *
 * **Features**:
 * - **Visual playhead line**: 2-pixel wide vertical line
 * - **Color-coded states**: Aqua (playback/stopped), Red (recording)
 * - **Drag-to-scrub**: Click and drag the playhead to seek through the timeline
 * - **Beat-based positioning**: Uses beat coordinates for stable position during BPM changes
 * - **30Hz update rate**: Smooth playhead movement during playback
 *
 * **Coordinate System**:
 * - Uses beat-based coordinates (not time-based) to maintain position during BPM changes
 * - Position calculated from transport time → beat → pixel conversion
 * - Transform: `x = (beat - viewStartBeat) * pixelsPerBeat`
 *
 * **Mouse Interaction**:
 * - **Hit testing**: Accepts mouse events within ±3 pixels of playhead position
 * - **Drag scrubbing**: Click and drag to seek to a new position
 * - **User dragging flag**: Sets transport.setUserDragging() to prevent conflicts
 *
 * **Integration**:
 * - Owned by TrackListComponent as an overlay
 * - Updated via timer callback at 30Hz during playback
 * - Responds to zoom changes via setPixelsPerBeat()
 * - Responds to scroll changes via setViewStartBeat()
 *
 * **Thread Safety**: Must be used from message thread (JUCE UI component).
 *
 * @see TrackListComponent for the parent component
 * @see EditViewState for coordinate system utilities
 */
class PlayheadComponent final : public juce::Component, juce::Timer
{
public:
    //==============================================================================
    // Construction

    /**
     * @brief Constructs the playhead component.
     *
     * Starts a 30Hz timer for smooth playhead updates during playback.
     *
     * @param edit Reference to the Tracktion Edit (provides transport, tempo sequence).
     * @param editViewState Reference to EditViewState (not currently used, may be removed).
     * @param appEngine Reference to AppEngine (provides recording state for color feedback).
     */
    PlayheadComponent (te::Edit& edit, EditViewState& editViewState, AppEngine& appEngine);

    //==============================================================================
    // Component Overrides

    /**
     * @brief Renders the playhead as a vertical line at the current transport position.
     *
     * **Visual Feedback**:
     * - **Red**: Recording is active (provides clear visual indication of capture in progress)
     * - **Aqua**: Normal playback or stopped (default playhead color)
     *
     * The 2-pixel wide line is drawn at the xPosition calculated in timerCallback().
     *
     * @param g Graphics context for rendering.
     */
    void paint (juce::Graphics& g) override;

    /**
     * @brief Custom hit test to accept mouse events only near the playhead line.
     *
     * Returns true if the mouse is within ±3 pixels of the playhead position,
     * allowing the user to click and drag the playhead for scrubbing.
     *
     * @param x Mouse X coordinate.
     * @param y Mouse Y coordinate (unused).
     * @return True if mouse is within hit test area.
     */
    bool hitTest (int x, int y) override;

    //==============================================================================
    // Mouse Event Handling

    /**
     * @brief Handles mouse down to initiate drag scrubbing.
     *
     * Sets the transport's user dragging flag to prevent playback conflicts.
     *
     * @param e Mouse event (unused).
     */
    void mouseDown (const juce::MouseEvent& e) override;

    /**
     * @brief Handles mouse drag to scrub through the timeline.
     *
     * Converts mouse X coordinate to beat position, then to time, and updates
     * the transport position. Triggers immediate timer callback for visual update.
     *
     * @param e Mouse event containing drag position.
     */
    void mouseDrag (const juce::MouseEvent& e) override;

    /**
     * @brief Handles mouse up to end drag scrubbing.
     *
     * Clears the transport's user dragging flag.
     *
     * @param e Mouse event (unused).
     */
    void mouseUp (const juce::MouseEvent& e) override;

    //==============================================================================
    // View Configuration

    /**
     * @brief Sets the horizontal zoom level for playhead positioning.
     *
     * Updates the pixels-per-beat scale used for coordinate conversion.
     * Clamped to minimum of 10.0 to prevent division by zero.
     *
     * @param p Pixels per beat (minimum 10.0).
     */
    void setPixelsPerBeat(double p) { pixelsPerBeat = juce::jmax(10.0, p); }

    /**
     * @brief Sets the leftmost visible beat position for playhead offset calculation.
     *
     * Updates the view start position used for coordinate conversion.
     *
     * @param b Beat position at the left edge of the visible area.
     */
    void setViewStartBeat(t::BeatPosition b) { viewStartBeat = b; }

private:
    //==============================================================================
    // Timer Callback

    /**
     * @brief Updates the playhead position during playback (called at 30Hz).
     *
     * Converts transport time → beat → pixel coordinate and triggers minimal repaint
     * if position has changed. Uses efficient repaint region (only the area between
     * old and new positions).
     */
    void timerCallback() override;

    //==============================================================================
    // Member Variables

    te::Edit& edit;                ///< Reference to Tracktion Edit (not owned).
    EditViewState& editViewState;  ///< Reference to EditViewState (not owned, currently unused).
    AppEngine& appEngine;          ///< Reference to AppEngine (not owned, used for recording state).

    int xPosition = 0;             ///< Current playhead X position in pixels.
    double pixelsPerBeat = 100.0;  ///< Horizontal zoom level (pixels per beat).
    t::BeatPosition viewStartBeat { t::BeatPosition::fromBeats(0.0) }; ///< Leftmost visible beat (scroll offset).

    bool firstTimer = true;        ///< Flag for first timer callback (internal state).
};
