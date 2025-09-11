//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackListComponent.h"
#include "../PopupWindows/PianoRollWindow.h"
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
    void paint (Graphics&) override;
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

    // Do we need this here?
    // EditViewState &editViewState;

    juce::TextButton backButton { "Back" }, newEditButton{"New"}, playPauseButton{"Play"}, stopButton{"Stop"},
            openEditButton{"Open Edit"}, newTrackButton{"New Track"}, recordButton{"Record"}, outputButton{"Output Device"},
            mixViewButton{"Mix View"};
};