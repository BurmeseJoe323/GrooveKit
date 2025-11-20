#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackClip.h"
#include "TrackHeaderComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
namespace t = tracktion;

/**
 * @brief Individual track lane displaying MIDI clips in horizontal timeline layout.
 *
 * TrackComponent represents a single track's visual lane in the track editor, containing
 * TrackClip UI components for each MIDI clip on that track. It acts as a listener for
 * TrackHeaderComponent button events (mute/solo/arm/delete) and propagates them to AppEngine.
 *
 * Architecture:
 *  - Owned by TrackListComponent (one TrackComponent per track)
 *  - Creates and owns TrackClip UI components via OwnedArray
 *  - Implements TrackHeaderComponent::Listener to handle header button clicks
 *  - Coordinates with parent for zoom (pixelsPerBeat) and scroll (viewStartBeat)
 *  - Uses beat-based coordinate system converted to pixels for rendering
 *
 * Clip Management:
 *  - rebuildClipsFromEngine(): Queries AppEngine for clips and recreates UI
 *  - Immediate mode: Always rebuilds from engine state, no local caching
 *  - Each TrackClip handles its own drag/resize interactions
 *  - Double-click on clip triggers piano roll editor via onRequestOpenPianoRoll callback
 *
 * Coordinate System:
 *  - pixelsPerBeat: Horizontal zoom level (pixels per beat)
 *  - viewStartBeat: Horizontal scroll position (beat at left edge)
 *  - Conversion helpers: timeToX(), xFromTime(), timeRangeToWidth()
 *
 * Event Handling:
 *  - Right-click shows track context menu (delete, settings, etc.)
 *  - Header button events routed through TrackHeaderComponent::Listener interface
 *  - Clip double-click handled by TrackClip with callback to parent
 *
 * Usage:
 *  - Created by TrackListComponent during rebuildFromEngine()
 *  - Call rebuildClipsFromEngine() when clips are added/removed/modified
 *  - Set callbacks (onRequestDeleteTrack, onRequestOpenPianoRoll, onRequestOpenDrumSampler)
 *  - Update zoom via setPixelsPerBeat(), scroll via setViewStartBeat()
 */
class TrackComponent final : public juce::Component, public TrackHeaderComponent::Listener
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs a track component.
     *
     * @param engine Shared pointer to AppEngine for track/clip access
     * @param trackIndex Index of this track in the Edit (0-based)
     * @param color Visual color for clip backgrounds
     */
    TrackComponent (const std::shared_ptr<AppEngine>& engine, int trackIndex, juce::Colour color);

    /** Destructor. */
    ~TrackComponent() override;

    //==============================================================================
    // TrackHeaderComponent::Listener Overrides

    /**
     * @brief Called when instrument button clicked on track header.
     *
     * Opens drum sampler or instrument plugin editor depending on track type.
     */
    void onInstrumentClicked() override;

    /**
     * @brief Called when changing active instrument.
     *
     * Opens the list of instruments.
     */
    void onInstrumentMenuRequested() override;

    /**
     * @brief Called when settings button clicked on track header.
     *
     * Shows track settings menu (currently unused).
     */
    void onSettingsClicked() override;

    /**
     * @brief Called when mute button toggled on track header.
     *
     * @param isMuted New mute state
     */
    void onMuteToggled (bool isMuted) override;

    /**
     * @brief Called when solo button toggled on track header.
     *
     * @param isSolo New solo state
     */
    void onSoloToggled (bool isSolo) override;

    /**
     * @brief Called when record arm button toggled on track header.
     *
     * @param isArmed New arm state
     */
    void onRecordArmToggled (bool isArmed) override;

    //==============================================================================
    // Track Index Management

    /**
     * @brief Sets the track index for this component.
     *
     * @param index Track index (0-based)
     */
    void setTrackIndex (int index);

    /**
     * @brief Returns the track index for this component.
     *
     * @return Track index (0-based)
     */
    int getTrackIndex() const;

    //==============================================================================
    // Clip Management

    /**
     * @brief Rebuilds all clip UI components from current engine state.
     *
     * Queries AppEngine for MIDI clips on this track, clears existing clip UI,
     * and creates new TrackClip components. Uses immediate mode rendering pattern.
     * Call this whenever clips are added, removed, or modified.
     */
    void rebuildClipsFromEngine();

    /**
     * @brief Updates visual state to highlight the clip being edited in piano roll.
     *
     * Sets the "edited" state on the specified clip, giving it a distinct visual
     * appearance. Clears edited state on all other clips.
     *
     * @param editedClip Pointer to clip being edited (nullptr to clear all highlights)
     */
    void updateClipEditedState (te::MidiClip* editedClip);

    //==============================================================================
    // Component Overrides

    void paint (juce::Graphics& g) override;
    void resized() override;

    /**
     * @brief Handles right-click to show track context menu.
     *
     * @param e Mouse event
     */
    void mouseUp (const juce::MouseEvent& e) override;

    //==============================================================================
    // Zoom and Scroll

    /**
     * @brief Sets the horizontal zoom level.
     *
     * @param ppb Pixels per beat
     */
    void setPixelsPerBeat (const double ppb)
    {
        pixelsPerBeat = ppb;
        resized();
    }

    /**
     * @brief Sets the horizontal scroll position.
     *
     * @param b Beat position to show at left edge
     */
    void setViewStartBeat (const t::BeatPosition b)
    {
        viewStartBeat = b;
        resized();
    }

    //==============================================================================
    // Callbacks

    std::function<void (int)> onRequestDeleteTrack; ///< Callback when user requests track deletion
    std::function<void (te::MidiClip* clip)> onRequestOpenPianoRoll; ///< Callback to open piano roll for clip
    std::function<void (int)> onRequestOpenDrumSampler; ///< Callback to open drum sampler editor

private:
    //==============================================================================
    // Member Variables

    std::shared_ptr<AppEngine> appEngine; ///< Shared pointer to global engine (non-owning wrapper)
    std::shared_ptr<MIDIEngine> midiEngine; ///< Shared pointer to MIDI engine (non-owning wrapper)
    juce::Colour trackColor; ///< Visual color for clip backgrounds
    int trackIndex = -1; ///< Track index in Edit (0-based)
    int numClips = 0; ///< Cached clip count (updated during rebuild)

    juce::OwnedArray<TrackClip> clipUIs; ///< Owned array of clip UI components

    double pixelsPerBeat = 100.0; ///< Horizontal zoom level (pixels per beat)
    t::BeatPosition viewStartBeat = t::BeatPosition::fromBeats(0.0); ///< Horizontal scroll position (beat at left edge)

    //==============================================================================
    // Coordinate Conversion Helpers

    /**
     * @brief Converts time position to X coordinate in pixels.
     *
     * @param t Time position to convert
     * @param view0 View start time
     * @param pps Pixels per second
     * @return X coordinate in pixels
     */
    static int timeToX (const t::TimePosition t, const t::TimePosition view0, const double pps)
    {
        return juce::roundToInt ((t - view0).inSeconds() * pps);
    }

    /**
     * @brief Converts time position to X coordinate (floor rounding).
     *
     * @param t Time position to convert
     * @param view0 View start time
     * @param pps Pixels per second
     * @return X coordinate in pixels (floored)
     */
    static int xFromTime (const t::TimePosition t, const t::TimePosition view0, const double pps)
    {
        const double secs = (t - view0).inSeconds();
        return static_cast<int> (std::floor (secs * pps));
    }

    /**
     * @brief Converts time range to width in pixels.
     *
     * @param r Time range
     * @param pps Pixels per second
     * @return Width in pixels
     */
    static int timeRangeToWidth (const t::TimeRange r, const double pps)
    {
        return juce::roundToInt (r.getLength().inSeconds() * pps);
    }

    //==============================================================================
    // Internal Methods

    /**
     * @brief Rebuilds clips and restores piano roll highlight.
     *
     * Helper method combining rebuildClipsFromEngine() with updateClipEditedState()
     * to maintain visual consistency when clips change.
     */
    void rebuildAndRefreshHighlight();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackComponent)
};
