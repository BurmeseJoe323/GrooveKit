#include "PluginEditorWindow.h"

using namespace juce;

std::unique_ptr<PluginEditorWindow>
PluginEditorWindow::createFor (AudioPluginInstance& instance, std::function<void()> onClose)
{
    return std::unique_ptr<PluginEditorWindow> (new PluginEditorWindow (instance, std::move (onClose)));
}

std::unique_ptr<PluginEditorWindow>
PluginEditorWindow::createFor (te::ExternalPlugin& ext, std::function<void()> onClose)
{
    if (auto* pi = ext.getAudioPluginInstance())
        return createFor (*pi, std::move (onClose));
    return {};
}

PluginEditorWindow::PluginEditorWindow (AudioPluginInstance& instance, std::function<void()> onClose)
    : DocumentWindow (instance.getName(), Colours::black, DocumentWindow::closeButton, true),
      inst (instance),
      onCloseCb (std::move (onClose))
{
    setUsingNativeTitleBar (true);

    Component* editor = nullptr;
    if (inst.hasEditor())
        editor = inst.createEditorIfNeeded();                 // owned by window after setContentOwned
    else
        editor = new GenericAudioProcessorEditor (inst);      // fallback for plugins without GUI

    jassert (editor != nullptr);
    setContentOwned (editor, true);

    const int w = jmax (editor->getWidth(),  520);
    const int h = jmax (editor->getHeight(), 360);

    centreWithSize (w, h);
    setResizable (true, true);
    setVisible (true);
    toFront (true);
}

PluginEditorWindow::~PluginEditorWindow()
{
    // Content is owned and will be deleted automatically
}

void PluginEditorWindow::closeButtonPressed()
{
    setVisible (false);
    if (onCloseCb) onCloseCb();
}
