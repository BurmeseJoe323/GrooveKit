//
// Created by Bracken Asay on 4/2/25.
//

#pragma once

#include "../../AppEngine/AppEngine.h"
#include "TrackListComponent.h"
#include "../PopupWindows/PianoRollWindow.h"

/**
 * Represents the track editor view, with functionality for adding and deleting tracks.
 * Each track contains a corresponding header, footer, and a series of MIDI clips.
 */
class TrackEditView final : public Component
{
public:
    explicit TrackEditView ();
    ~TrackEditView() override;

    void setupButtons();
    void paint (Graphics&) override;
    void resized() override;

private:
    std::shared_ptr<AppEngine> appEngine;
    std::unique_ptr<TrackListComponent> trackList;
    std::unique_ptr<PianoRollWindow> pianoRollWindow;
    Viewport viewport;

    TextButton newEditButton{"New"}, playPauseButton{"Play"}, stopButton{"Stop"},
            openEditButton{"Open Edit"}, newTrackButton{"New Track"}, recordButton{"Record"};
};