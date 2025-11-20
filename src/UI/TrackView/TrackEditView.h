// Created by Bracken Asay on 4/2/25.
// Note: Junie (JetBrains AI) contributed code to this file on 2025-09-24.

#pragma once

#include "../../AppEngine/AppEngine.h"
#include "../PopupWindows/PianoRollComponents/PianoRollEditor.h"
#include "TrackListComponent.h"
#include "ExportOverlayComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace te = tracktion::engine;
namespace t = tracktion;

class AppEngine;
class TransportBar;
class GrooveKitMenuBar;

/**
 * @brief Main track editor view with transport controls and MIDI recording UI.
 *
 * TrackEditView provides the primary sequencer interface, including:
 *  - Transport controls (play, stop, record)
 *  - BPM editing and metronome toggle
 *  - Track list display with scrolling viewport
 *  - Piano roll editor for MIDI clip editing
 *  - Menu bar for file and track operations
 *
 * The view inherits from juce::Timer to update UI state in real-time, such as
 * the record button visual feedback during recording.
 *
 * Recording workflow:
 *  1. User arms a track (handled by TrackHeaderComponent)
 *  2. User clicks the record button
 *  3. Timer callback updates button appearance to indicate recording state
 *  4. User clicks record button again to stop
 */
class TrackEditView final : public juce::Component, juce::Timer
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the Track Edit view with shared components.
     *
     * @param engine Reference to AppEngine for track/clip access
     * @param transport Reference to shared TransportBar component
     * @param menuBar Reference to shared GrooveKitMenuBar component
     */
    explicit TrackEditView (AppEngine& engine, TransportBar& transport, GrooveKitMenuBar& menuBar);

    /** Destructor. */
    ~TrackEditView() override;

    //==============================================================================
    // Component Overrides

    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    // Keyboard and Mouse Handlers

    /**
     * @brief Handles keyboard presses for MIDI playback via QWERTY keyboard.
     *
     * Maps Q-I keys to MIDI notes C-C for live performance input.
     * Spacebar toggles transport play/stop.
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
    bool keyStateChanged(bool isKeyDown) override;

    //==============================================================================
    // Piano Roll Management

    /**
     * @brief Shows the piano roll editor for the given MIDI clip.
     *
     * Opens the resizable piano roll panel below the track list, allowing
     * note editing for the specified clip. Updates menu bar callbacks to
     * ensure piano roll closes when tracks are added.
     *
     * @param clip Pointer to MidiClip to edit (must be valid)
     */
    void showPianoRoll (te::MidiClip* clip);

    /**
     * @brief Hides the piano roll editor and returns to track-only view.
     *
     * Collapses the piano roll panel and clears the edited clip reference.
     */
    void hidePianoRoll();

    /**
     * @brief Restores clip highlight state after track rebuild.
     *
     * When tracks are rebuilt (e.g., after adding/deleting tracks), this method
     * re-applies the visual highlight to the currently edited clip in the piano roll.
     */
    void refreshClipEditState();

    /**
     * @brief Returns the track index of the clip currently being edited in piano roll.
     *
     * @return Track index, or -1 if piano roll not visible
     */
    int getPianoRollIndex() const;

    //==============================================================================
    // Callbacks

    std::function<void()> onBack; ///< Callback to navigate back to home screen
    std::function<void()> onOpenMix; ///< Callback to switch to Mix view

    /**
     * @brief Timer callback for UI state updates (100ms interval).
     *
     * Updates the track color with a red tint when recording
     *
     * This provides real-time visual indication of recording status without
     * requiring manual UI refresh calls from the recording subsystem.
     */
    void timerCallback() override;

    //==============================================================================
    // Nested Classes

    /**
     * @brief Custom resizer bar for adjusting piano roll height.
     *
     * Extends StretchableLayoutResizerBar to provide visual feedback and
     * handle drag events for resizing the piano roll panel.
     */
    class PianoRollResizerBar final : public juce::StretchableLayoutResizerBar
    {
    public:
        /**
         * @brief Constructs the resizer bar.
         *
         * @param layoutToUse Pointer to the layout manager
         * @param itemIndexInLayout Index of the resizable item
         * @param isBarVertical True if vertical orientation
         */
        PianoRollResizerBar (juce::StretchableLayoutManager* layoutToUse, int itemIndexInLayout, bool isBarVertical);

        /** Destructor. */
        ~PianoRollResizerBar() override;

        /**
         * @brief Called when the resizer bar has been moved.
         */
        void hasBeenMoved() override;

        /**
         * @brief Handles mouse drag events for resizing.
         *
         * @param event Mouse event
         */
        void mouseDrag (const juce::MouseEvent& event) override;
    };

private:
    //==============================================================================
    // Component Overrides (Private)

    /**
     * @brief Updates menu bar mode when view becomes visible.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Ensures focus for keyboard input.
     *
     * @param event Mouse event
     */
    void mouseDown (const juce::MouseEvent&) override;

    //==============================================================================
    // Member Variables

    std::shared_ptr<AppEngine> appEngine; ///< Shared pointer to global engine (non-owning wrapper)
    TransportBar* transportBar; ///< Non-owning pointer to shared transport bar
    GrooveKitMenuBar* menuBar; ///< Non-owning pointer to shared menu bar

    std::unique_ptr<TrackListComponent> trackList; ///< Owned track list with timeline and clips
    juce::Viewport viewport; ///< Scrollable viewport containing track list

    std::unique_ptr<PianoRollEditor> pianoRoll; ///< Owned piano roll editor (created on demand)
    juce::StretchableLayoutManager verticalLayout; ///< Layout manager for piano roll resizing
    std::unique_ptr<PianoRollResizerBar> resizerBar; ///< Owned resizer bar between track list and piano roll
    te::MidiClip* pianoRollClip = nullptr; ///< Currently edited clip in piano roll (not owned)
    int pianoRollTrackIndex = -1; ///< Track index of currently edited clip
    bool pianoRollVisible = false; ///< Whether piano roll is currently visible

    std::unique_ptr<ExportOverlayComponent> exportOverlay;

    double pixelsPerBeat = 100.0; ///< Horizontal zoom level (pixels per beat)
    t::BeatPosition viewStartBeat = t::BeatPosition::fromBeats(0.0); ///< Horizontal scroll position

    juce::TextButton backButton { "Back" }, newEditButton { "New" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, outputButton { "Output Device" },
        mixViewButton { "Mix View" }; ///< Legacy UI buttons (deprecated)
    juce::TextButton loopButton { "loop" }; ///< Legacy loop button (deprecated)

    bool wasRecording = false;  ///< Track previous recording state to detect changes

};