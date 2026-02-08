#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "MixerPanel.h"
#include "../../AppEngine/AppEngine.h"

class TransportBar;
class GrooveKitMenuBar;

/**
 * @brief Mix view providing mixer-style track controls with faders, pan, and effects.
 *
 * MixView presents a traditional DAW mixer interface with vertical fader strips for each track.
 * It provides access to volume, pan, mute, solo controls and displays the track's plugin chain.
 * This view complements TrackEditView by focusing on mixing rather than MIDI editing.
 *
 * Architecture:
 *  - Owns a MixerPanel which creates MixerStrip components for each track
 *  - Shares TransportBar and GrooveKitMenuBar with TrackEditView (non-owning pointers)
 *  - Handles MIDI keyboard input (QWERTY and physical MIDI) for live playback
 *  - Updates menu bar to Mix view mode when becoming visible
 *
 * Usage:
 *  - Created by MainComponent alongside TrackEditView
 *  - Call refreshMixer() when tracks are added/removed to rebuild mixer strips
 *  - Set onBack callback to handle user navigation back to Track Edit view
 */
class MixView : public juce::Component
{
public:
    //==============================================================================
    // Type Aliases

    using ToggleTracksCallback = std::function<void()>;

    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the Mix view with shared components.
     *
     * @param engine Reference to AppEngine for track/playback access
     * @param transport Reference to shared TransportBar component
     * @param menuBar Reference to shared GrooveKitMenuBar component
     */
    MixView(AppEngine& engine, TransportBar& transport, GrooveKitMenuBar& menuBar);

    /** Destructor. */
    ~MixView() override;

    //==============================================================================
    // Component Overrides

    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Keyboard and Mouse Handlers

    /**
     * @brief Handles keyboard presses for MIDI playback via QWERTY keyboard.
     *
     * Maps Q-I keys to MIDI notes C-C for live performance input.
     * Injects MIDI into currently armed track.
     */
    bool keyPressed(const juce::KeyPress&) override;

    /**
     * @brief Handles key state changes for note-off events.
     *
     * @param isKeyDown True if any key is currently pressed
     */
    bool keyStateChanged(bool isKeyDown) override;

    /**
     * @brief Updates menu bar mode when view becomes visible.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Ensures focus for keyboard input.
     */
    void mouseDown(const juce::MouseEvent&) override;

    //==============================================================================
    // Public API

    /**
     * @brief Rebuilds mixer strips from current engine track state.
     *
     * Call this when tracks are added/removed/reordered to refresh the mixer display.
     */
    void refreshMixer() { if (mixerPanel) mixerPanel->refreshTracks(); }

    //==============================================================================
    // Callbacks

    std::function<void()> onBack; ///< Callback to navigate back to Track Edit view

private:
    //==============================================================================
    // Member Variables

    AppEngine& appEngine; ///< Reference to global engine (not owned)
    TransportBar* transportBar; ///< Non-owning pointer to shared transport bar
    GrooveKitMenuBar* menuBar; ///< Non-owning pointer to shared menu bar
    std::unique_ptr<MixerPanel> mixerPanel; ///< Owned mixer panel containing track strips

    int outerMargin = 16; ///< Margin around mixer panel in pixels
};

