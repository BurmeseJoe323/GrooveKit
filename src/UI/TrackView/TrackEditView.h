// Created by Bracken Asay on 4/2/25.
// Note: Junie (JetBrains AI) contributed code to this file on 2025-09-24.

#pragma once

#include "../../AppEngine/AppEngine.h"
#include "../PopupWindows/PianoRollComponents/PianoRollEditor.h"
#include "TrackListComponent.h"
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
    explicit TrackEditView (AppEngine& engine, TransportBar& transport, GrooveKitMenuBar& menuBar);
    ~TrackEditView() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;
    bool keyStateChanged(bool isKeyDown) override;

    /**
     * Called when Back is pressed. Should return to home screen.
     */
    std::function<void()> onBack;
    std::function<void()> onOpenMix;

    void showPianoRoll (te::MidiClip* clip);
    void hidePianoRoll();
    void refreshClipEditState(); // Restore clip highlight after rebuild (Written by Claude Code)

    int getPianoRollIndex() const;

    class PianoRollResizerBar final : public juce::StretchableLayoutResizerBar
    {
    public:
        PianoRollResizerBar (juce::StretchableLayoutManager* layoutToUse, int itemIndexInLayout, bool isBarVertical);
        ~PianoRollResizerBar() override;

        void hasBeenMoved() override;
        void mouseDrag (const juce::MouseEvent& event) override;
    };

private:
    std::shared_ptr<AppEngine> appEngine;
    TransportBar* transportBar;
    GrooveKitMenuBar* menuBar;

    std::unique_ptr<TrackListComponent> trackList;
    juce::Viewport viewport;

    std::unique_ptr<PianoRollEditor> pianoRoll;
    juce::StretchableLayoutManager verticalLayout;
    std::unique_ptr<PianoRollResizerBar> resizerBar;
    te::MidiClip* pianoRollClip = nullptr; // currently edited clip (if any)
    int pianoRollTrackIndex = -1;
    bool pianoRollVisible = false;

    double pixelsPerBeat = 100.0;
    t::BeatPosition viewStartBeat = t::BeatPosition::fromBeats(0.0);

    void parentHierarchyChanged() override;
    void mouseDown (const juce::MouseEvent&) override;

    /**
     * @brief Timer callback for UI state updates (100ms interval).
     *
     * Updates the track color with a red tint when recording
     *
     * This provides real-time visual indication of recording status without
     * requiring manual UI refresh calls from the recording subsystem.
     */
    void timerCallback() override;

    juce::TextButton backButton { "Back" }, newEditButton { "New" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, outputButton { "Output Device" },
        mixViewButton { "Mix View" };
    juce::TextButton loopButton { "loop" };

    bool wasRecording = false;  ///< Track previous recording state to detect changes

};