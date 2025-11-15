#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "MixerPanel.h"
#include "../../AppEngine/AppEngine.h"

class MixView : public juce::Component
{
public:
    using ToggleTracksCallback = std::function<void()>;

    MixView(AppEngine& engine);
    ~MixView();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Keyboard event handlers for MIDI playback (Written by Claude Code)
    bool keyPressed(const juce::KeyPress&) override;
    bool keyStateChanged(bool isKeyDown) override;
    void parentHierarchyChanged() override;
    void mouseDown(const juce::MouseEvent&) override;

    void refreshMixer() { if (mixerPanel) mixerPanel->refreshTracks(); }

    std::function<void()> onBack;

private:
    AppEngine& appEngine;
    juce::TextButton backButton;
    std::unique_ptr<MixerPanel> mixerPanel;

    int outerMargin = 16;
    int topBarHeight = 48;
};

