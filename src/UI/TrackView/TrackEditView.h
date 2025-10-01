// Note: Junie (JetBrains AI) contributed code to this file on 2025-09-24.
#pragma once

#include "../../AppEngine/AppEngine.h"
#include "../PopupWindows/PianoRollWindow.h"
#include "TrackListComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

class AppEngine;

/**
 * Represents the track editor view, with functionality for adding and deleting tracks.
 * Each track contains a corresponding header, footer, and a series of MIDI clips.
 */
class TrackEditView final : public juce::Component, juce::KeyListener, public juce::MidiKeyboardStateListener
{
public:
    explicit TrackEditView (AppEngine& engine);
    ~TrackEditView() override;

    void setupButtons();
    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&, juce::Component *) override;

    // MidiKeyboardStateListener
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override;

    /**
      Called when Back is pressed: should return to home screen.
    */
    std::function<void()> onBack;
    std::function<void()> onOpenMix;

private:
    void injectNoteMessage (const juce::MidiMessage& msg);

    std::shared_ptr<AppEngine> appEngine;
    std::unique_ptr<TrackListComponent> trackList;
    std::unique_ptr<PianoRollWindow> pianoRollWindow;
    juce::Viewport viewport;
    double pixelsPerSecond = 100.0;
    te::TimePosition viewStart = 0s;

    // Shared MidiKeyboardState for receiving/performance note events.
    juce::MidiKeyboardState midiKeyboardState;

    juce::TextButton backButton { "Back" }, newEditButton { "New" }, playPauseButton { "Play" }, stopButton { "Stop" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, recordButton { "Record" }, outputButton { "Output Device" },
        mixViewButton { "Mix View" };
};