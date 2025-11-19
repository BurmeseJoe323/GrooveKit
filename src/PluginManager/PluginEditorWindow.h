#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * @brief Hosts and displays the GUI editor for an external plugin.
 *
 * PluginEditorWindow wraps a plugin's editor (custom or generic) in a
 * JUCE DocumentWindow. It:
 *  - Creates and owns the plugin editor Component.
 *  - Forwards keyboard events to an optional KeyListener (e.g. for MIDI).
 *  - Invokes a callback when the window is closed.
 *
 * Use the static createFor() helpers to construct an instance.
 */
class PluginEditorWindow : public juce::DocumentWindow
{
public:
    //==============================================================================
    /**
     * @brief Creates a window for a raw AudioPluginInstance.
     *
     * @param instance   The plugin instance whose editor should be displayed.
     * @param onClose    Callback fired when the window is closed (optional).
     * @param keyForward Optional key listener to forward key events to.
     *
     * @return A unique_ptr to a PluginEditorWindow, or nullptr if creation fails.
     */
    static std::unique_ptr<PluginEditorWindow>
    createFor (juce::AudioPluginInstance& instance,
               std::function<void()> onClose = {},
               juce::KeyListener* keyForward = nullptr);

    /**
     * @brief Creates a window for a Tracktion ExternalPlugin.
     *
     * If the ExternalPlugin has a valid AudioPluginInstance, this behaves
     * like the other createFor() overload. Otherwise, returns nullptr.
     *
     * @param ext        External plugin wrapper from Tracktion Engine.
     * @param onClose    Callback fired when the window is closed (optional).
     * @param keyForward Optional key listener to forward key events to.
     *
     * @return A unique_ptr to a PluginEditorWindow, or nullptr on failure.
     */
    static std::unique_ptr<PluginEditorWindow>
    createFor (te::ExternalPlugin& ext,
               std::function<void()> onClose = {},
               juce::KeyListener* keyForward = nullptr);

    /** @brief Destructor. Releases the plugin editor Component. */
    ~PluginEditorWindow() override;

    //==============================================================================
    // juce::DocumentWindow overrides

    /** @brief Called when the window's close button is pressed. */
    void closeButtonPressed() override;

    /**
     * @brief Handles key presses and optionally forwards them to keyForwarder.
     */
    bool keyPressed (const juce::KeyPress&) override;

    /**
     * @brief Handles key state changes and optionally forwards them to keyForwarder.
     */
    bool keyStateChanged (bool isKeyDown) override;

private:
    //==============================================================================
    /**
     * @brief Private constructor – use the createFor() factory methods.
     *
     * @param instance     The plugin instance to display.
     * @param onClose      Callback fired when the window closes.
     * @param keyForward   Optional key listener used (e.g.) to feed keys into a MIDI keyboard.
     */
    PluginEditorWindow (juce::AudioPluginInstance& instance,
                        std::function<void()> onClose,
                        juce::KeyListener* keyForward);

    //==============================================================================
    // Member variables

    juce::AudioPluginInstance& inst;    ///< The audio plugin instance being displayed.
    std::function<void()>      onCloseCb; ///< Client-provided on-close callback.
    juce::KeyListener*         keyForwarder; ///< Key-forwarding target (may be nullptr).
};
