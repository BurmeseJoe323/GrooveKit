#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class DrumPadComponent : public juce::Component,
                         public juce::FileDragAndDropTarget,
                         public juce::DragAndDropTarget
{
public:
    using OnDropFile   = std::function<void (const juce::File&)>;
    using OnTrigger    = std::function<void (float velocity)>;

    DrumPadComponent (int slotIndex, OnDropFile onDrop, OnTrigger onTrig)
        : slot (slotIndex), onDropFile (std::move (onDrop)), onTrigger (std::move (onTrig))
    {
        setInterceptsMouseClicks (true, true);
        setRepaintsOnMouseActivity (true);
    }

    void setTitle (juce::String s) { title = std::move (s); repaint(); }

    bool isInterestedInFileDrag (const juce::StringArray& files) override
    {
        return files.size() > 0;
    }

    void filesDropped (const juce::StringArray& files, int, int) override
    {
        if (files.isEmpty()) return;
        juce::File f (files[0]);
        if (onDropFile) onDropFile (f);
    }

    bool isInterestedInDragSource (const SourceDetails& d) override
    {
        return d.description.isString(); // we pass absolute file path as a string
    }

    void itemDropped (const SourceDetails& d) override
    {
        DBG ("[Pad] dropped: " << d.description.toString());
        const juce::String path = d.description.toString();
        const juce::File   f (path);
        if (f.existsAsFile() && onDropFile)
            onDropFile (f);
    }

    void itemDragEnter (const SourceDetails&) override { flashOn = true; repaint(); }
    void itemDragExit  (const SourceDetails&) override { flashOn = false; repaint(); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const float v = juce::jlimit (0.1f, 1.0f, 1.0f - (float)e.position.y / (float)getHeight());
        if (onTrigger) onTrigger (v);
        flashOn = true; repaint();
    }
    void mouseUp (const juce::MouseEvent&) override
    {
        flashOn = false; repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();
        g.setColour (flashOn ? juce::Colours::orange : juce::Colours::darkslategrey);
        g.fillRoundedRectangle (r, 10.0f);

        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.drawRoundedRectangle (r.reduced (1.5f), 10.0f, 2.0f);

        juce::Font font (13.0f); font.setBold (true);
        g.setFont (font);

        g.drawFittedText (title.isNotEmpty() ? title : juce::String("Pad " + juce::String(slot)),
                          getLocalBounds().reduced (6), juce::Justification::centred, 2);
    }

private:
    int slot;
    bool flashOn = false;
    juce::String title;

    OnDropFile onDropFile;
    OnTrigger  onTrigger;
};

