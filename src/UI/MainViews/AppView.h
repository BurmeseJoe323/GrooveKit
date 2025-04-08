#ifndef APPVIEW_H
#define APPVIEW_H

#include "../../AppEngine/AppEngine.h"
#include "TrackView.h"
#include <vector>

#include "../PopupWindows/PianoRollWindow.h"

class AppView : public juce::Component
{
public:
    AppView();
    ~AppView() override = default;
    void resized() override;

    void paint(juce::Graphics& g) override;
    void closePianoRoll();

private:
    std::unique_ptr<TrackView> trackView;
    std::unique_ptr<PianoRollWindow> pianoRollWindow;

    AppEngine engine;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton addTrackButton;
    juce::TextButton addClipButton;
    juce::TextButton openPianoRollWindow;

    int clipCount = 0;
    int trackCount = 0;
    int selectedTrackIndex = 0;

    juce::TextButton openTrackView;

    void openPianoRoll();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppView)
};
#endif