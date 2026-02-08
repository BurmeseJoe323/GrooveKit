#pragma once

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @brief A single drum pad UI element with click + drag-and-drop behavior.
 *
 * DrumPadComponent represents one pad in the 4x4 drum grid. It:
 *  - Triggers a callback with a velocity when clicked.
 *  - Accepts external files via drag-and-drop.
 *  - Accepts internal drag sources whose description is a file path string.
 *  - Flashes visually while being clicked or dragged over.
 *
 * Audio playback is handled elsewhere (via the callbacks), so this component
 * is purely a UI + event emitter.
 */
class DrumPadComponent : public juce::Component,
                         public juce::FileDragAndDropTarget,
                         public juce::DragAndDropTarget
{
public:
    //==============================================================================
    /** Callback type used when a sample file is dropped onto the pad. */
    using OnDropFile = std::function<void (const juce::File&)>;

    /** Callback type used when the pad is triggered (e.g., by a mouse click). */
    using OnTrigger  = std::function<void (float velocity)>;

    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs a pad for the given slot index.
     *
     * @param slotIndex  Index of this pad (e.g., 0–15).
     * @param onDrop     Called when a file is dropped on this pad.
     * @param onTrig     Called when the pad is triggered with a velocity.
     */
    DrumPadComponent (int slotIndex, OnDropFile onDrop, OnTrigger onTrig)
        : slot (slotIndex),
          onDropFile (std::move (onDrop)),
          onTrigger (std::move (onTrig))
    {
        setInterceptsMouseClicks (true, true);
        setRepaintsOnMouseActivity (true);
    }

    //==============================================================================
    // Public API

    /**
     * @brief Sets the text label displayed on the pad (usually the sample name).
     */
    void setTitle (juce::String s)
    {
        title = std::move (s);
        repaint();
    }

    //==============================================================================
    // juce::FileDragAndDropTarget

    bool isInterestedInFileDrag (const juce::StringArray& files) override
    {
        return files.size() > 0;
    }

    void filesDropped (const juce::StringArray& files, int, int) override
    {
        if (files.isEmpty())
            return;

        const juce::File f (files[0]);
        if (onDropFile)
            onDropFile (f);
    }

    //==============================================================================
    // juce::DragAndDropTarget

    bool isInterestedInDragSource (const SourceDetails& d) override
    {
        // We pass absolute file paths as strings when dragging from the sample list.
        return d.description.isString();
    }

    void itemDropped (const SourceDetails& d) override
    {
        const juce::File f (d.description.toString());
        if (f.existsAsFile() && onDropFile)
            onDropFile (f);
    }

    void itemDragEnter (const SourceDetails&) override
    {
        flashOn = true;
        repaint();
    }

    void itemDragExit (const SourceDetails&) override
    {
        flashOn = false;
        repaint();
    }

    //==============================================================================
    // Mouse interaction

    void mouseDown (const juce::MouseEvent& e) override
    {
        // Map vertical click position to a simple velocity curve (top = 1.0, bottom = 0.1).
        const float v = juce::jlimit (0.1f, 1.0f,
                                      1.0f - (float) e.position.y / (float) getHeight());

        if (onTrigger)
            onTrigger (v);

        flashOn = true;
        repaint();
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        flashOn = false;
        repaint();
    }

    //==============================================================================
    // Painting

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();

        // Background
        g.setColour (flashOn ? juce::Colours::orange
                             : juce::Colours::darkslategrey);
        g.fillRoundedRectangle (r, 10.0f);

        // Border
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.drawRoundedRectangle (r.reduced (1.5f), 10.0f, 2.0f);

        // Label
        juce::Font font (juce::FontOptions (13.0f));
        font.setBold (true);
        g.setFont (font);

        const juce::String labelText =
            title.isNotEmpty() ? title : ("Pad " + juce::String (slot));

        g.drawFittedText (labelText,
                          getLocalBounds().reduced (6),
                          juce::Justification::centred,
                          2);
    }

private:
    //==============================================================================
    int slot;
    bool flashOn = false;
    juce::String title;

    OnDropFile onDropFile;
    OnTrigger  onTrigger;
};