#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/** Hosts a plugin editor (native if provided, else Generic).
    Optionally forwards key events to a KeyListener (e.g., your MidiListener)
    so QWERTY works even when the editor has focus.
*/
class PluginEditorWindow : public juce::DocumentWindow
{
public:
    static std::unique_ptr<PluginEditorWindow>
    createFor(juce::AudioPluginInstance& instance,
              std::function<void()> onClose = {},
              juce::KeyListener* keyForward = nullptr);

    static std::unique_ptr<PluginEditorWindow>
    createFor(te::ExternalPlugin& ext,
              std::function<void()> onClose = {},
              juce::KeyListener* keyForward = nullptr);

    ~PluginEditorWindow() override;

    void closeButtonPressed() override;
    bool keyPressed(const juce::KeyPress&) override;
    bool keyStateChanged(bool isKeyDown) override;

private:
    PluginEditorWindow(juce::AudioPluginInstance& instance,
                       std::function<void()> onClose,
                       juce::KeyListener* keyForward);

    juce::AudioPluginInstance& inst;
    std::function<void()> onCloseCb;
    juce::KeyListener* keyForwarder = nullptr;
};