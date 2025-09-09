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

    // ===== FileDragAndDropTarget (external files from Finder/Explorer)
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

    // ===== DragAndDropTarget (internal drags from our library list)
    bool isInterestedInDragSource (const SourceDetails& dragSourceDetails) override
    {
        juce::ignoreUnused (dragSourceDetails);
        return true; // accept anything; we’ll validate in itemDropped
    }

    void itemDropped (const SourceDetails& dragSourceDetails) override
    {
        // Expect the description to be a file path string
        if (dragSourceDetails.description.isString())
        {
            juce::File f (dragSourceDetails.description.toString());
            if (f.existsAsFile() && onDropFile)
                onDropFile (f);
        }
    }

    void itemDragEnter (const SourceDetails&) override { flashOn = true; repaint(); }
    void itemDragExit  (const SourceDetails&) override { flashOn = false; repaint(); }

    // ===== UI
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

        g.drawFittedText (title.isNotEmpty() ? title : "Pad " + juce::String (slot + 1),
                          getLocalBounds().reduced (6), juce::Justification::centred, 2);
    }

private:
    int slot;
    bool flashOn = false;
    juce::String title;

    OnDropFile onDropFile;
    OnTrigger  onTrigger;
};

