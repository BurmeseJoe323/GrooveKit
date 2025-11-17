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
 * Represents the track editor view, with functionality for adding and deleting tracks.
 * Each track contains a corresponding header, footer, and a series of MIDI clips.
 */
class TrackEditView final : public juce::Component
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

    std::unique_ptr<ExportOverlayComponent> exportOverlay;

    double pixelsPerBeat = 100.0;
    t::BeatPosition viewStartBeat = t::BeatPosition::fromBeats(0.0);

    // --- Top Bar Components ---
    std::unique_ptr<juce::MenuBarComponent> menuBar;

    // Center controls
    juce::Label bpmLabel, bpmEditField;
    juce::ShapeButton playButton { "play", {}, {}, {} };
    juce::ShapeButton stopButton { "stop", {}, {}, {} };
    juce::ShapeButton recordButton { "record", {}, {}, {} };

    // Right side (placeholder)
    juce::TextButton switchButton { "|||" };

    // Private helper methods for menu actions
    void showOutputDeviceSettings() const;
    void showMidiInputDeviceSettings() const;
    void showNewEditMenu() const;
    void showOpenEditMenu() const;
    void exportAudio();

    void parentHierarchyChanged() override;
    void mouseDown (const juce::MouseEvent&) override;

    juce::TextButton backButton { "Back" }, newEditButton { "New" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, outputButton { "Output Device" },
        mixViewButton { "Mix View" };
    juce::TextButton loopButton { "loop" };

};