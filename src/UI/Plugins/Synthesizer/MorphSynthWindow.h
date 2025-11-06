#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "MorphSynthView.h"

// ==============================================================================
// MorphSynthWindow.h
// ------------------------------------------------------------------------------
// Top-level window that hosts the MorphSynthView UI.
//
// Responsibilities:
//  - Creates and owns the editor component (MorphSynthView).
//  - Manages basic window chrome (native title bar, resize, centre).
//  - Notifies an external closer via callback when the window is closed.
// ==============================================================================

/**
 * @brief DocumentWindow that wraps the Morph Synth editor.
 *
 * The window constructs a MorphSynthView as its content component and exposes
 * a close callback so the owner (e.g., AppEngine) can safely destroy it.
 */
class MorphSynthWindow : public juce::DocumentWindow
{
public:
    //==============================================================================
    /**
     * @brief Construct the window and show the MorphSynthView.
     * @param plugin    Reference to the plugin edited by the view.
     * @param onCloseFn Callback invoked when the close button is pressed.
     */
    explicit MorphSynthWindow (MorphSynthPlugin& plugin, std::function<void()> onCloseFn)
        : juce::DocumentWindow ("MorphSynth",
                                juce::Colours::darkgrey,
                                juce::DocumentWindow::closeButton),
          onClose (std::move (onCloseFn))
    {
        setUsingNativeTitleBar (true);
        setContentOwned (new MorphSynthView (plugin), true);
        centreWithSize (640, 800);
        setResizable (true, true);
        setVisible (true);

        juce::MessageManager::callAsync(
        [safe = juce::Component::SafePointer<MorphSynthWindow>(this)]
        {
            if (safe != nullptr && safe->isShowing())
                if (auto* cc = safe->getContentComponent())
                    cc->grabKeyboardFocus();
        });
        }

    //==============================================================================
    /** Close button handler; either invokes the external callback or hides. */
    void closeButtonPressed() override
    {
        // Decide how to close. If you manage this window with a unique_ptr in AppEngine,
        // call back so AppEngine can reset it safely.
        if (onClose) onClose();
        else         setVisible (false); // fallback
    }

    bool keyStateChanged (bool isKeyDown) override
    {
        if (appEngine)
        {
            auto& ml = appEngine->getMidiListener();
            return ml.handleKeyStateChanged(isKeyDown);
        }
        return false;
    }

    bool keyPressed (const juce::KeyPress& key) override
    {
        // If we have a MIDI listener, let it try first
        if (appEngine)
        {
            auto& ml = appEngine->getMidiListener();
            if (ml.handleKeyPress (key))
                return true; // consume the key so macOS won't beep
        }

        // If the key is one of the note keys, consume it even if held down
        const juce::String noteKeys = "awsedftgyhujkolp;"; // adjust to match your mapping
        if (noteKeys.containsChar (juce::CharacterFunctions::toLowerCase (key.getTextCharacter())))
            return true; // prevent "no key handler" beep on auto-repeat

        return false; // not handled → let normal JUCE shortcuts work
    }

    void setAppEngine(std::shared_ptr<AppEngine> e) { appEngine = std::move(e); }
private:
    //==============================================================================
    std::shared_ptr<AppEngine> appEngine;
    std::function<void()> onClose; ///< External close callback (may be null)
};