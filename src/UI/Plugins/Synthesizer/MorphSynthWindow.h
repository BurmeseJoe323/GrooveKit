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
    explicit MorphSynthWindow (MorphSynthPlugin& plugin,
                               std::function<void()> onCloseFn,
                               juce::KeyListener* keyForwarder = nullptr)
        : juce::DocumentWindow ("MorphSynth",
                                juce::Colours::darkgrey,
                                juce::DocumentWindow::closeButton),
          onClose (std::move (onCloseFn)),
          keyForward (keyForwarder)
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
        if (onClose) onClose();
        else         setVisible (false); // fallback
    }

    bool keyPressed (const juce::KeyPress& kp) override
    {
        // forward to the client (AppEngine’s adapter) first
        if (keyForward != nullptr && keyForward->keyPressed (kp, this))
            return true;

        // swallow note keys so macOS doesn’t beep on repeat
        const juce::String noteKeys = "awsedftgyhujkolp;";
        if (noteKeys.containsChar (juce::CharacterFunctions::toLowerCase (kp.getTextCharacter())))
            return true;

        return DocumentWindow::keyPressed (kp);
    }

    bool keyStateChanged (bool isDown) override
    {
        if (keyForward != nullptr && keyForward->keyStateChanged (isDown, this))
            return true;

        return DocumentWindow::keyStateChanged (isDown);
    }

    void setAppEngine(std::shared_ptr<AppEngine> e) { appEngine = std::move(e); }
private:
    //==============================================================================
    std::shared_ptr<AppEngine> appEngine;
    std::function<void()> onClose;
    juce::KeyListener* keyForward = nullptr;
};