#pragma once

#include "../../AppEngine/AppEngine.h"
#include "ChannelComponents/ChannelStrip.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace te = tracktion::engine;

/**
 * @brief Mixer panel containing vertical channel strips for tracks and master output.
 *
 * MixerPanel creates a traditional DAW mixer layout with scrollable ChannelStrip components.
 * Each strip displays volume fader, pan control, mute/solo/arm buttons, and plugin chain.
 * The master strip is fixed on the right side while track strips scroll horizontally.
 *
 * Architecture:
 *  - Owns OwnedArray of ChannelStrip components (one per track plus master)
 *  - Uses Viewport for horizontal scrolling of track strips
 *  - Queries AppEngine for track list and creates corresponding UI strips
 *  - Updates automatically when tracks are added/removed via refreshTracks()
 *
 * Usage:
 *  - Created and owned by MixView
 *  - Call refreshTracks() when track configuration changes
 *  - Call refreshArmStates() when armed track changes to update visual indicators
 */
class MixerPanel final : public juce::Component
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the mixer panel.
     *
     * @param engine Reference to AppEngine for track access
     */
    explicit MixerPanel (AppEngine& engine);

    /** Destructor. */
    ~MixerPanel() override;

    //==============================================================================
    // Public API

    /**
     * @brief Rebuilds all channel strips from current engine track state.
     *
     * Clears existing strips and creates new ChannelStrip components for each track
     * plus the master strip. Call this when tracks are added, removed, or reordered.
     */
    void refreshTracks();

    /**
     * @brief Updates armed state indicators on all channel strips.
     *
     * Iterates through strips and highlights the currently armed track.
     * Call this when the armed track changes.
     */
    void refreshArmStates();

    //==============================================================================
    // Component Overrides

    void resized() override;

private:
    //==============================================================================
    // Member Variables

    AppEngine& appEngine; ///< Reference to global engine (not owned)

    juce::OwnedArray<ChannelStrip> trackStrips; ///< Owned channel strips for tracks
    std::unique_ptr<ChannelStrip> masterStrip; ///< Owned master channel strip

    juce::Viewport tracksViewport; ///< Horizontal scrolling viewport for track strips
    juce::Component tracksContainer; ///< Container holding all track strips for viewport

    int innerMargin = 12; ///< Internal margin around strips in pixels
    int gap = 12; ///< Gap between channel strips in pixels
    int stripW = 120; ///< Width of each channel strip in pixels
};
