#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class SampleLibraryComponent : public juce::Component,
                               public juce::FileDragAndDropTarget,
                               public juce::ListBoxModel
{
public:
    explicit SampleLibraryComponent (juce::DragAndDropContainer* dndContainerIn)
        : dndContainer (dndContainerIn)
    {
        addAndMakeVisible (listBox);
        listBox.setModel (this);
        listBox.setRowHeight (24);
        listBox.setMultipleSelectionEnabled (false);

        addAndMakeVisible (addButton);
        addButton.setButtonText ("+");
        addButton.onClick = [this] { openChooser(); };
    }

    void addFile (juce::File f)
    {
        if (f.existsAsFile())
        {
            for (auto& existing : samples)
                if (existing == f)
                    return; // already in library

            samples.add (std::move (f));
            listBox.updateContent();
            repaint();
        }
    }

    // ===== FileDragAndDropTarget
    bool isInterestedInFileDrag (const juce::StringArray& files) override { return files.size() > 0; }
    void filesDropped (const juce::StringArray& files, int, int) override
    {
        for (auto& path : files)
            addFile (juce::File (path));
    }

    // ===== ListBoxModel
    int getNumRows() override { return samples.size(); }

    void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool isSelected) override
    {
        juce::ignoreUnused (isSelected);
        g.fillAll (row % 2 ? juce::Colours::black.withAlpha (0.06f)
                           : juce::Colours::transparentBlack);

        if (juce::isPositiveAndBelow (row, samples.size()))
        {
            g.setColour (juce::Colours::white);
            g.drawText (samples[row].getFileNameWithoutExtension(),
                        8, 0, w - 16, h, juce::Justification::centredLeft, false);
        }
    }

    void listBoxItemClicked (int row, const juce::MouseEvent& e) override
    {
        if (! juce::isPositiveAndBelow (row, samples.size())) return;

        if (e.mods.isLeftButtonDown() && dndContainer != nullptr)
        {
            const juce::File& f = samples[row];

            juce::Image dragImg (juce::Image::ARGB, 160, 22, true);
            {
                juce::Graphics gg (dragImg);
                gg.setColour (juce::Colours::darkgrey);
                gg.fillRoundedRectangle (dragImg.getBounds().toFloat(), 4.0f);
                gg.setColour (juce::Colours::white);
                gg.drawText (f.getFileName(), dragImg.getBounds().reduced (6, 2),
                             juce::Justification::centredLeft, false);
            }

            // Use ScaledImage overload (no deprecation, correct signature)
            juce::ScaledImage scaledDrag (dragImg, 1.0f);
            dndContainer->startDragging (f.getFullPathName(), this, scaledDrag);
        }
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6);
        auto top = r.removeFromTop (28);
        addButton.setBounds (top.removeFromRight (28));
        listBox.setBounds (r);
    }

private:
    void openChooser()
    {
        auto chooser = std::make_shared<juce::FileChooser>(
            "Add samples", juce::File{}, "*.wav;*.aif;*.aiff;*.flac;*.mp3");

        chooser->launchAsync (juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectFiles
                            | juce::FileBrowserComponent::canSelectMultipleItems,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto results = fc.getResults(); // JUCE 8 return-by-value
                for (auto& f : results)
                    addFile (f);
            });
    }

    juce::ListBox listBox { "SampleLibrary", this };
    juce::TextButton addButton;
    juce::Array<juce::File> samples;
    juce::DragAndDropContainer* dndContainer = nullptr;
};

