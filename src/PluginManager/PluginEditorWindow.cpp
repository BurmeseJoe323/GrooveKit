#include "PluginEditorWindow.h"

using namespace juce;

//==============================================================================
// Factory methods

std::unique_ptr<PluginEditorWindow>
PluginEditorWindow::createFor (AudioPluginInstance& instance,
                               std::function<void()> onClose,
                               KeyListener* keyForward)
{
    return std::unique_ptr<PluginEditorWindow> (
        new PluginEditorWindow (instance, std::move (onClose), keyForward));
}

std::unique_ptr<PluginEditorWindow>
PluginEditorWindow::createFor (te::ExternalPlugin& ext,
                               std::function<void()> onClose,
                               KeyListener* keyForward)
{
    if (auto* pi = ext.getAudioPluginInstance())
        return createFor (*pi, std::move (onClose), keyForward);

    return {};
}

//==============================================================================
// Construction

PluginEditorWindow::PluginEditorWindow (AudioPluginInstance& instance,
                                        std::function<void()> onClose,
                                        KeyListener* keyForward)
    : DocumentWindow (instance.getName(),
                      Colours::black,
                      DocumentWindow::closeButton,
                      true),
      inst (instance),
      onCloseCb (std::move (onClose)),
      keyForwarder (keyForward)
{
    setUsingNativeTitleBar (true);

    // Create plugin editor UI (custom editor if available, otherwise generic).
    Component* editor = nullptr;
    if (inst.hasEditor())
        editor = inst.createEditorIfNeeded();
    else
        editor = new GenericAudioProcessorEditor (inst);

    jassert (editor != nullptr);
    setContentOwned (editor, true);

    // Size & layout.
    int w = editor->getWidth();
    int h = editor->getHeight();

    if (w <= 0) w = 400;
    if (h <= 0) h = 300;

    centreWithSize (w, h);

    setResizable (true, true);
    setVisible (true);
    toFront (true);

    // Ensure the window receives keyboard focus.
    setWantsKeyboardFocus (true);
    grabKeyboardFocus ();

    // Attach QWERTY key listener (MidiListenerKeyAdapter) directly to this window
    // and the editor component so it sees key events while the plugin is open.
    if (keyForwarder != nullptr)
    {
        addKeyListener (keyForwarder);

        if (auto* c = getContentComponent())
            c->addKeyListener (keyForwarder);
    }

    // Let the editor request keyboard focus as well.
    if (auto* c = getContentComponent())
        c->setWantsKeyboardFocus (true);
}

PluginEditorWindow::~PluginEditorWindow()
{
    // Detach key listener to avoid dangling pointers.
    if (keyForwarder != nullptr)
    {
        removeKeyListener (keyForwarder);

        if (auto* c = getContentComponent())
            c->removeKeyListener (keyForwarder);
    }
}

//==============================================================================
// Window behavior

void PluginEditorWindow::closeButtonPressed()
{
    setVisible (false);

    if (onCloseCb)
        onCloseCb();
}

//==============================================================================
// Keyboard event forwarding

bool PluginEditorWindow::keyPressed (const juce::KeyPress& key)
{
    // Forward to the adapter if it exists, using this window as the originating component.
    if (keyForwarder != nullptr)
    {
        if (keyForwarder->keyPressed (key, this))
            return true; // event handled by the adapter
    }

    return DocumentWindow::keyPressed (key);
}

bool PluginEditorWindow::keyStateChanged (bool isDown)
{
    // Forward state changes too.
    if (keyForwarder != nullptr)
    {
        if (keyForwarder->keyStateChanged (isDown, this))
            return true;
    }

    return DocumentWindow::keyStateChanged (isDown);
}
