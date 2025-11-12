#include "PluginEditorWindow.h"
using namespace juce;

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

    Component* editor = nullptr;
    if (inst.hasEditor())
        editor = inst.createEditorIfNeeded();
    else
        editor = new GenericAudioProcessorEditor(inst);

    jassert(editor != nullptr);
    setContentOwned(editor, true);

    const int w = jmax(editor->getWidth(),  520);
    const int h = jmax(editor->getHeight(), 360);
    centreWithSize(w, h);

    setResizable(true, true);
    setVisible(true);
    toFront(true);

    // Make sure we actually receive key events
    setWantsKeyboardFocus(true);
    grabKeyboardFocus();

    // Also let content bubble keys up to us
    if (auto* c = getContentComponent())
        c->setWantsKeyboardFocus(true);
}

PluginEditorWindow::~PluginEditorWindow() = default;

void PluginEditorWindow::closeButtonPressed()
{
    setVisible(false);
    if (onCloseCb) onCloseCb();
}

bool PluginEditorWindow::keyPressed (const KeyPress& kp)
{
    // Forward to client listener (e.g., MidiListener) first
    if (keyForwarder != nullptr && keyForwarder->keyPressed(kp, this))
        return true;

    // Let plugin/editor handle it next
    return DocumentWindow::keyPressed(kp);
}

bool PluginEditorWindow::keyStateChanged (bool isDown)
{
    if (keyForwarder != nullptr && keyForwarder->keyStateChanged(isDown, this))
        return true;

    return DocumentWindow::keyStateChanged(isDown);
}

