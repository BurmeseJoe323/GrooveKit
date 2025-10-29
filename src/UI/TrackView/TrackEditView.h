// Created by Bracken Asay on 4/2/25.
// Note: Junie (JetBrains AI) contributed code to this file on 2025-09-24.

#pragma once

#include "../../AppEngine/AppEngine.h"
#include "../PopupWindows/PianoRollComponents/PianoRollEditor.h"
#include "TrackListComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;
class MidiListener;

/**
 * Represents the track editor view, with functionality for adding and deleting tracks.
 * Each track contains a corresponding header, footer, and a series of MIDI clips.
 */
class TrackEditView final : public juce::Component, public juce::MenuBarModel, public juce::Label::Listener
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

    void showPianoRoll(te::MidiClip* clip);
    void hidePianoRoll();

    int getPianoRollIndex() const;

    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    /**
     * Returns a reference to the MidiListener that manages MIDI keyboard input.
     */
    MidiListener& getMidiListener() const { return *midiListener; }

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

    // MIDI listener handles keyboard input and routing to tracks
    std::unique_ptr<MidiListener> midiListener;

    std::unique_ptr<PianoRollEditor> pianoRoll;
    juce::StretchableLayoutManager verticalLayout;
    std::unique_ptr<PianoRollResizerBar> resizerBar;
    int pianoRollTrackIndex = -1;
    bool pianoRollVisible = false;

    double pixelsPerSecond = 100.0;
    te::TimePosition viewStart = 0s;

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
    void showNewEditMenu() const;
    void showOpenEditMenu() const;

    juce::TextButton backButton { "Back" }, newEditButton { "New" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, outputButton { "Output Device" },
        mixViewButton { "Mix View" };
    juce::TextButton loopButton { "loop" };

};