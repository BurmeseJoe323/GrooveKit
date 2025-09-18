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

    // --- Top Bar Components --- A native-style menu bar.
    class MainMenuModel; // Forward-declare the model
    std::unique_ptr<MainMenuModel> menuModel;
    std::unique_ptr<juce::MenuBarComponent> menuBar;

    // Center controls
    juce::Label bpmLabel, clickLabel;
    juce::ShapeButton playButton { "play", {}, {}, {} };
    juce::ShapeButton stopButton { "stop", {}, {}, {} };
    juce::ShapeButton recordButton { "record", {}, {}, {} };

    // Right side (placeholder)
    juce::TextButton switchButton { "|||" };

    // Private helper methods for menu actions
    void showNewTrackMenu();
    void showOutputDeviceSettings();

    // Friend class to allow the menu model to call private methods
    friend class MainMenuModel;
};