//
// Created by Joseph Rockwell on 4/10/25.
//

#ifndef TIMELINECOMPONENT_H
#define TIMELINECOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 * @brief Horizontal timeline ruler displaying bar/beat markers for the piano roll.
 *
 * TimelineComponent renders a visual timeline ruler at the top of the piano roll
 * editor, showing bar numbers and beat divisions for temporal navigation. It
 * synchronizes with the NoteGridComponent's horizontal zoom and scroll position.
 *
 * **Features:**
 * - Displays bar numbers (1, 2, 3, etc.)
 * - Shows beat subdivisions within each bar
 * - Configurable zoom level (pixels per bar)
 * - Horizontal scrolling synchronized with note grid
 * - Visual time reference for MIDI editing
 *
 * **Coordinate System:**
 * - X-axis: Time (left = bar 1, right extends based on barsToDraw)
 * - Each bar occupies pixelsPerBar width
 * - Major tick marks at bar boundaries
 * - Minor tick marks at beat subdivisions (4/4 time signature assumed)
 *
 * **Configuration:**
 * Call setup() to configure the timeline's range and zoom level. This is typically
 * called when the piano roll editor is initialized or when zoom changes.
 *
 * **Integration:**
 * Used by PianoRollEditor as the top ruler component. Width must match
 * NoteGridComponent for proper alignment. Scrolling is managed by parent component.
 *
 * @note Currently assumes 4/4 time signature. Future versions may support dynamic
 *       time signature configuration via commented-out constructor.
 *
 * @see PianoRollEditor
 * @see NoteGridComponent
 * @see PRE::defaultResolution
 */
class TimelineComponent : public juce::Component
{
public:
    //==============================================================================
    // Lifecycle

    /**
     * @brief Constructs the timeline with default (unconfigured) state.
     *
     * Call setup() after construction to configure bar range and zoom level.
     */
    TimelineComponent();

    //TimelineComponent (const timeSig); // Future: constructor with time signature support

    //==============================================================================
    // Configuration

    /**
     * @brief Configures the timeline's range and zoom level.
     *
     * Sets the number of bars to display and the horizontal zoom factor. This
     * determines the component's required width and tick mark spacing.
     *
     * @param barsToDraw Total number of bars to display (e.g., 8 for 8-bar loop)
     * @param pixelsPerBar Horizontal zoom factor in pixels per bar (e.g., 120)
     */
    void setup (const int barsToDraw, const int pixelsPerBar);

    //==============================================================================
    // Component Overrides

    /**
     * @brief Renders the timeline ruler with bar/beat markers.
     *
     * Draws:
     * - Bar numbers at major tick marks
     * - Vertical lines at bar boundaries
     * - Beat subdivision markers
     * - Background fill
     *
     * @param g Graphics context for rendering
     */
    void paint (juce::Graphics& g);

    /**
     * @brief Called when component is resized (currently unused).
     *
     * Reserved for future layout adjustments.
     */
    void resized();

private:
    //==============================================================================
    // Private Members

    int barsToDraw;   ///< Number of bars to display in the timeline
    int pixelsPerBar; ///< Horizontal zoom factor (pixels per bar)
};

#endif //TIMELINECOMPONENT_H
