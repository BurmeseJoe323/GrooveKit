#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/** A simple window that hosts a plugin's editor (native if available,
    otherwise a GenericAudioProcessorEditor).
    Works with any juce::AudioPluginInstance (VST3/AU/etc).
*/
class PluginEditorWindow : public juce::DocumentWindow
{
public:
    /** Create a window for a concrete instance. The instance must outlive this window. */
    static std::unique_ptr<PluginEditorWindow>
    createFor (juce::AudioPluginInstance& instance, std::function<void()> onClose = {});

    /** Convenience: open from an ExternalPlugin; returns nullptr if there is no instance yet. */
    static std::unique_ptr<PluginEditorWindow>
    createFor (te::ExternalPlugin& ext, std::function<void()> onClose = {});

    ~PluginEditorWindow() override;

    void closeButtonPressed() override;

private:
    PluginEditorWindow (juce::AudioPluginInstance& instance, std::function<void()> onClose);

    juce::AudioPluginInstance& inst;
    std::function<void()> onCloseCb;
};
