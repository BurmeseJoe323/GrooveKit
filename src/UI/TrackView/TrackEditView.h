#pragma once

#include "../../AppEngine/AppEngine.h"
#include "../PopupWindows/PianoRollWindow.h"
#include "TrackListComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

class AppEngine;

/**
 * Represents the track editor view, with functionality for adding and deleting tracks.
 * Each track contains a corresponding header, footer, and a series of MIDI clips.
 */
class TrackEditView final : public juce::Component
{
public:
    explicit TrackEditView (AppEngine& engine);
    ~TrackEditView() override;

    void setupButtons();
    void paint (juce::Graphics&) override;
    void resized() override;

    /**
      Called when Back is pressed: should return to home screen.
    */
    std::function<void()> onBack;
    std::function<void()> onOpenMix;

private:
    std::shared_ptr<AppEngine> appEngine;
    std::unique_ptr<TrackListComponent> trackList;
    std::unique_ptr<PianoRollWindow> pianoRollWindow;
    juce::Viewport viewport;
    double pixelsPerSecond = 100.0;
    te::TimePosition viewStart = 0s;

    juce::TextButton backButton { "Back" }, newEditButton { "New" }, playPauseButton { "Play" }, stopButton { "Stop" },
        openEditButton { "Open Edit" }, newTrackButton { "New Track" }, recordButton { "Record" }, outputButton { "Output Device" },
        mixViewButton { "Mix View" };
};