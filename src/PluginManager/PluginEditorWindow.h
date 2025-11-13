#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * A window that hosts and displays the GUI editor for an external plugin.
 *
 * Allows opening plugin UIs (AU, VST3, etc.) in a simple
 * JUCE DocumentWindow and handles keyboard forwarding for MIDI input and
 * on-close callbacks for lifecycle control.
 */
class PluginEditorWindow : public juce::DocumentWindow
{
public:
    //==============================================================================
    /** Creates a window for a raw AudioPluginInstance. */
    static std::unique_ptr<PluginEditorWindow>
    createFor(juce::AudioPluginInstance& instance,
              std::function<void()> onClose = {},
              juce::KeyListener* keyForward = nullptr);

    /** Creates a window for a Tracktion ExternalPlugin. */
    static std::unique_ptr<PluginEditorWindow>
    createFor(te::ExternalPlugin& ext,
              std::function<void()> onClose = {},
              juce::KeyListener* keyForward = nullptr);

    ~PluginEditorWindow() override;

    //==============================================================================
    // DocumentWindow overrides
    void closeButtonPressed() override;
    bool keyPressed(const juce::KeyPress&) override;
    bool keyStateChanged(bool isKeyDown) override;

private:
    //==============================================================================
    /**
     * Private constructor – use createFor().
     *
     * @param instance     The plugin instance to display
     * @param onClose      Callback fired when the window closes
     * @param keyForward   Optional key listener used (e.g.) to feed keys into a MIDI keyboard
     */
    PluginEditorWindow(juce::AudioPluginInstance& instance,
                       std::function<void()> onClose,
                       juce::KeyListener* keyForward);

    //==============================================================================
    juce::AudioPluginInstance& inst;    ///< The audio plugin instance being displayed
    std::function<void()> onCloseCb;    ///< Client-provided on-close callback
    juce::KeyListener* keyForwarder;    ///< Key-forwarding target
};