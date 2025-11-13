#include "PluginEditorWindow.h"
using namespace juce;

//==============================================================================
// Factory methods

std::unique_ptr<PluginEditorWindow>
PluginEditorWindow::createFor(AudioPluginInstance& instance,
                              std::function<void()> onClose,
                              KeyListener* keyForward)
{
    return std::unique_ptr<PluginEditorWindow>(
        new PluginEditorWindow(instance, std::move(onClose), keyForward));
}

std::unique_ptr<PluginEditorWindow>
PluginEditorWindow::createFor(te::ExternalPlugin& ext,
                              std::function<void()> onClose,
                              KeyListener* keyForward)
{
    if (auto* pi = ext.getAudioPluginInstance())
        return createFor(*pi, std::move(onClose), keyForward);

    return {};
}

//==============================================================================
// Construction

PluginEditorWindow::PluginEditorWindow(AudioPluginInstance& instance,
                                       std::function<void()> onClose,
                                       KeyListener* keyForward)
    : DocumentWindow(instance.getName(),
                     Colours::black,
                     DocumentWindow::closeButton,
                     true),
      inst(instance),
      onCloseCb(std::move(onClose)),
      keyForwarder(keyForward)
{
    setUsingNativeTitleBar(true);

    // Create plugin editor UI (custom editor if available, otherwise generic)
    Component* editor = nullptr;
    if (inst.hasEditor())
        editor = inst.createEditorIfNeeded();
    else
        editor = new GenericAudioProcessorEditor(inst);

    jassert(editor != nullptr);
    setContentOwned(editor, true);

    // Size & layout
    const int w = jmax(editor->getWidth(),  520);
    const int h = jmax(editor->getHeight(), 360);
    centreWithSize(w, h);

    setResizable(true, true);
    setVisible(true);
    toFront(true);

    // Ensure the window receives keyboard focus
    setWantsKeyboardFocus(true);
    grabKeyboardFocus();

    // Let the editor request keyboard focus
    if (auto* c = getContentComponent())
        c->setWantsKeyboardFocus(true);
}

PluginEditorWindow::~PluginEditorWindow() = default;

//==============================================================================
// Window behavior

void PluginEditorWindow::closeButtonPressed()
{
    setVisible(false);
    if (onCloseCb)
        onCloseCb();
}

//==============================================================================
// Keyboard event forwarding

bool PluginEditorWindow::keyPressed(const KeyPress& kp)
{
    // First allow external listener to consume the key if desired
    if (keyForwarder != nullptr && keyForwarder->keyPressed(kp, this))
        return true;

    // Otherwise let JUCE/DocumentWindow handle it
    return DocumentWindow::keyPressed(kp);
}

bool PluginEditorWindow::keyStateChanged(bool isDown)
{
    if (keyForwarder != nullptr && keyForwarder->keyStateChanged(isDown, this))
        return true;

    return DocumentWindow::keyStateChanged(isDown);
}