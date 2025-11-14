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
class TrackEditView final : public juce::Component, public juce::MenuBarModel, public juce::Label::Listener, private juce::Timer
{
public:
    explicit TrackEditView (AppEngine& engine);
    ~TrackEditView() override;

    void setupButtons();
    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;
    bool keyStateChanged(bool isKeyDown) override;

    // --- MenuBarModel overrides ---
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex (int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;

    /**
     * Called when Back is pressed. Should return to home screen.
     */
    std::function<void()> onBack;
    std::function<void()> onOpenMix;

    void showPianoRoll (te::MidiClip* clip);
    void hidePianoRoll();

    int getPianoRollIndex() const;

    void labelTextChanged(juce::Label* labelThatHasChanged) override;

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

    // --- Top Bar Components ---
    std::unique_ptr<juce::MenuBarComponent> menuBar;

    // Center controls
    juce::Label bpmLabel, bpmEditField;
    juce::ShapeButton playButton { "play", {}, {}, {} };
    juce::ShapeButton stopButton { "stop", {}, {}, {} };
    juce::ShapeButton recordButton { "record", {}, {}, {} };
    juce::ToggleButton metronomeButton { "Click" };

    // Right side (placeholder)
    juce::TextButton switchButton { "|||" };

    // Private helper methods for menu actions
    void showOutputDeviceSettings() const;
    void showNewEditMenu() const;
    void showOpenEditMenu() const;

    void parentHierarchyChanged() override;
    void mouseDown (const juce::MouseEvent&) override;

    /**
     * @brief Timer callback for UI state updates (100ms interval).
     *
     * Updates the record button appearance based on the current recording state:
     *  - Recording: Brighter red color for visual feedback
     *  - Not recording: Normal red color
     *
     * This provides real-time visual indication of recording status without
     * requiring manual UI refresh calls from the recording subsystem.
     */
    void timerCallback() override;

    juce::TextButton backButton { "Back" }, newEditButton { "New" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, outputButton { "Output Device" },
        mixViewButton { "Mix View" };
    juce::TextButton loopButton { "loop" };

};