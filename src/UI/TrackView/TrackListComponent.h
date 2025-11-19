#pragma once

#include "../../AppEngine/AppEngine.h"
#include "PlayheadComponent.h"
#include "LoopRangeComponent.h"
#include "TrackComponent.h"
#include "TrackHeaderComponent.h"
#include "GhostClipComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "TimelineComponent.h"

namespace te = tracktion::engine;
namespace t = tracktion;

/**
 * @brief Container component holding timeline, track headers, track lanes, and playback indicators.
 *
 * TrackListComponent is the core track editing surface in GrooveKit's TrackEditView. It manages
 * the layout and interaction between multiple UI components:
 *  - TimelineComponent (beat/bar ruler at top)
 *  - TrackHeaderComponent array (track names, mute/solo/arm buttons on left)
 *  - TrackComponent array (MIDI clip lanes on right)
 *  - PlayheadComponent (vertical playback position indicator)
 *  - LoopRangeComponent (visual loop range overlay)
 *
 * Architecture:
 *  - Owned by TrackEditView (wrapped in Viewport for scrolling)
 *  - Rebuilds track UI via rebuildFromEngine() when track configuration changes
 *  - Coordinates zoom (pixelsPerBeat) and scroll (viewStartBeat) across all child components
 *  - Uses OwnedArray for automatic memory management of dynamic track UI elements
 *
 * Clip Drag System:
 *  - Validates clip movement between tracks via canClipMoveToTrack()
 *  - Detects overlaps with wouldClipOverlap() to prevent invalid drop positions
 *  - Shows ghost clip preview during drag operations
 *  - Converts screen Y coordinates to track indices for drop targets
 *
 * Track Rebuild System:
 *  - rebuildFromEngine(): Full rebuild of all tracks (called on track add/delete)
 *  - rebuildTrack(): Incremental rebuild of single track (called on clip changes)
 *  - updateClipEditState(): Restores piano roll highlight after rebuild
 *
 * Usage:
 *  - Call rebuildFromEngine() after adding/deleting tracks
 *  - Call setPixelsPerBeat() to update zoom level
 *  - Call setViewStartBeat() to update horizontal scroll position
 *  - Use getTrackIndexAtY() to convert mouse Y to track index for drag/drop
 */
class TrackListComponent final : public juce::Component
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the track list component.
     *
     * @param engine Shared pointer to AppEngine for track access
     */
    explicit TrackListComponent (const std::shared_ptr<AppEngine>& engine);

    /** Destructor. */
    ~TrackListComponent() override;

    //==============================================================================
    // Component Overrides

    void paint (juce::Graphics& g) override;
    void resized() override;
    void parentSizeChanged() override;

    //==============================================================================
    // Keyboard and Mouse Handlers

    /**
     * @brief Handles keyboard input for MIDI playback and transport control.
     *
     * @param key KeyPress event
     * @return true if handled
     */
    bool keyPressed (const juce::KeyPress&) override;

    /**
     * @brief Handles key state changes for note-off events.
     *
     * @param isKeyDown True if any key is currently pressed
     * @return true if handled
     */
    bool keyStateChanged (bool isKeyDown) override;

    /**
     * @brief Ensures focus for keyboard input.
     *
     * @param event Mouse event
     */
    void mouseDown (const juce::MouseEvent&) override;

    /**
     * @brief Updates focus when view becomes visible.
     */
    void parentHierarchyChanged() override;

    //==============================================================================
    // Track Management

    /**
     * @brief Refreshes mute/solo/arm button states from engine.
     *
     * Updates visual state of all track header buttons to match AppEngine state.
     */
    void refreshTrackStates() const;

    /**
     * @brief Adds a new track UI at the given index (deprecated).
     *
     * Use rebuildFromEngine() instead for reliable track UI updates.
     *
     * @param index Track index to add
     */
    void addNewTrack (int index);

    /**
     * @brief Rebuilds all track UI components from engine state.
     *
     * Clears existing tracks/headers and recreates UI for all tracks in the Edit.
     * Call this after adding/deleting tracks, or when track configuration changes.
     */
    void rebuildFromEngine();

    /**
     * @brief Rebuilds UI for a single track.
     *
     * Incremental update for when only one track's clips have changed.
     * More efficient than full rebuild when adding/removing clips.
     *
     * @param trackIndex Index of track to rebuild
     */
    void rebuildTrack (int trackIndex);

    /**
     * @brief Updates clip highlight state after rebuild.
     *
     * Restores the visual highlight on the clip currently being edited in piano roll.
     * Called after rebuildTrack() to maintain UI consistency.
     *
     * @param trackIndex Track containing the edited clip
     * @param editedClip Pointer to the clip being edited (not owned)
     */
    void updateClipEditState (int trackIndex, te::MidiClip* editedClip);

    /**
     * @brief Arms or disarms a track for recording.
     *
     * @param trackIndex Track index to arm
     * @param shouldBeArmed True to arm, false to disarm
     */
    void armTrack(int trackIndex, bool shouldBeArmed);
    void setAllArmButtonsEnabled(bool enabled);
    void repaintTrack(int trackIndex);

    //==============================================================================
    // Zoom and Scroll

    /**
     * @brief Sets the horizontal zoom level.
     *
     * Updates all child components (timeline, tracks) to use new zoom.
     *
     * @param ppb Pixels per beat
     */
    void setPixelsPerBeat (double ppb);

    /**
     * @brief Sets the horizontal scroll position.
     *
     * Updates all child components to display the new scroll position.
     *
     * @param b Beat position to show at left edge
     */
    void setViewStartBeat (t::BeatPosition b);

    /**
     * @brief Returns the current horizontal zoom level.
     *
     * @return Pixels per beat
     */
    double getPixelsPerBeat() const
    {
        return timeline ? timeline->getPixelsPerBeat() : 100.0;
    }

    /**
     * @brief Returns the current horizontal scroll position.
     *
     * @return Beat position at left edge of view
     */
    t::BeatPosition getViewStartBeat() const
    {
        return timeline ? timeline->getViewStartBeat() : t::BeatPosition::fromBeats(0.0);
    }

    //==============================================================================
    // Clip Drag Validation

    /**
     * @brief Checks if a clip can be moved to a different track.
     *
     * Validates track type compatibility (drum clips can only move to drum tracks).
     *
     * @param clip Clip to validate
     * @param sourceTrack Source track index
     * @param targetTrack Target track index
     * @return true if move is valid
     */
    bool canClipMoveToTrack (te::MidiClip* clip, int sourceTrack, int targetTrack) const;

    /**
     * @brief Checks if a clip would overlap existing clips at target position.
     *
     * @param clipToMove Clip to check
     * @param targetTrack Target track index
     * @param range Time range of the moved clip
     * @return true if overlap would occur
     */
    bool wouldClipOverlap (te::MidiClip* clipToMove, int targetTrack, t::TimeRange range) const;

    /**
     * @brief Converts Y coordinate to track index.
     *
     * Used for drag/drop to determine which track the mouse is over.
     *
     * @param y Y coordinate in component space
     * @return Track index, or -1 if not over a track
     */
    int getTrackIndexAtY (int y) const;

    //==============================================================================
    // Ghost Clip (Drag Preview)

    /**
     * @brief Shows a ghost clip preview during drag operations.
     *
     * @param trackIndex Track index to show ghost on
     * @param time Start time of ghost clip
     * @param length Length of ghost clip
     * @param isValid True if drop position is valid (affects visual style)
     */
    void showGhostClip (int trackIndex, t::TimePosition time, t::TimeDuration length, bool isValid);

    /**
     * @brief Hides the ghost clip preview.
     */
    void hideGhostClip();

private:
    //==============================================================================
    // Member Variables

    const std::shared_ptr<AppEngine> appEngine; ///< Shared pointer to global engine (non-owning wrapper)

    std::unique_ptr<ui::TimelineComponent> timeline; ///< Owned timeline ruler component
    static constexpr int timelineHeight = 24; ///< Height of timeline in pixels
    static constexpr int headerWidth    = 140; ///< Width of track headers in pixels

    PlayheadComponent playhead; ///< Playback position indicator
    LoopRangeComponent loopRangeComponent; ///< Visual loop range overlay

    juce::OwnedArray<TrackComponent> tracks; ///< Owned track lane components (MIDI clips)
    juce::OwnedArray<TrackHeaderComponent> headers; ///< Owned track header components (buttons/names)
    juce::Array<juce::Colour> trackColors {
        juce::Colour::fromString ("#ff6b6b"),
        juce::Colour::fromString ("#f06595"),
        juce::Colour::fromString ("#cc5de8"),
        juce::Colour::fromString ("#845ef7"),
        juce::Colour::fromString ("#5c7cfa"),
        juce::Colour::fromString ("#339af0"),
        juce::Colour::fromString ("#22b8cf"),
        juce::Colour::fromString ("#20c997"),
        juce::Colour::fromString ("#51cf66"),
        juce::Colour::fromString ("#fcc419")
    }; ///< Color palette for track headers (cycles by index)
    juce::TextButton loopButton { "loop" }; ///< Legacy loop button (deprecated)

    std::unique_ptr<GhostClipComponent> ghostClip; ///< Drag preview component (created on demand)

    //==============================================================================
    // Internal Methods

    /**
     * @brief Updates track index values in all headers.
     *
     * Ensures each header knows its position in the track list for callbacks.
     */
    void updateTrackIndexes() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackListComponent)
};