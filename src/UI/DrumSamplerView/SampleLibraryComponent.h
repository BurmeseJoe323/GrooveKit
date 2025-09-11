#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "DrumSamplerEngine/DefaultSampleLibrary.h"
#include <memory>

class SampleRow : public juce::Component
{
public:
    explicit SampleRow (const juce::File& f) : file(f) {}

    void paint (juce::Graphics& g) override
    {
        g.fillAll (isMouseOver() ? juce::Colours::black.withAlpha(0.2f)
                                 : juce::Colours::transparentBlack);
        g.setColour (juce::Colours::white);
        g.setFont   (juce::Font (13.0f, juce::Font::bold));
        g.drawFittedText (file.getFileNameWithoutExtension(),
                          getLocalBounds().reduced(8, 2), juce::Justification::centredLeft, 1);
        g.setColour (juce::Colours::white.withAlpha(0.3f));
        g.drawLine  (0.0f, (float)getHeight() - 0.5f, (float)getWidth(), (float)getHeight() - 0.5f);
    }

    void mouseDrag (const juce::MouseEvent&) override
    {
        if (! hasStartedDrag)
            if (auto* dnd = juce::DragAndDropContainer::findParentDragContainerFor (this))
            {
                hasStartedDrag = true;
                dnd->startDragging (file.getFullPathName(), this); // description = absolute path
            }
    }

    void mouseUp (const juce::MouseEvent&) override { hasStartedDrag = false; }

    const juce::File& getFile() const { return file; }

private:
    juce::File file;
    bool hasStartedDrag = false;
};

class SampleListModel : public juce::ListBoxModel
{
public:
    void setFiles (juce::Array<juce::File> newFiles)
    {
        files.swapWith (newFiles);
    }

    int getNumRows() override { return files.size(); }

    void paintListBoxItem (int, juce::Graphics&, int, int, bool) override {}

    juce::Component* refreshComponentForRow (int row, bool, juce::Component* existing) override
    {
        if (! juce::isPositiveAndBelow (row, files.size())) return nullptr;
        auto* rowComp = dynamic_cast<SampleRow*> (existing);
        if (rowComp == nullptr || rowComp->getFile() != files[row])
        {
            delete existing;
            rowComp = new SampleRow (files[row]);
        }
        return rowComp;
    }

private:
    juce::Array<juce::File> files;
};

class SampleLibraryComponent : public juce::Component
{
public:
    SampleLibraryComponent()
    {
        DefaultSampleLibrary::ensureInstalled();
        refreshList();
        addAndMakeVisible (container);
        addAndMakeVisible (addButton);
        addAndMakeVisible (searchBox);
        addAndMakeVisible (list);

        addButton.setButtonText("+");
        addButton.setTooltip("Add sample(s) to library");
        addButton.onClick = [this]{ openChooser(); };

        searchBox.setTextToShowWhenEmpty ("Filter samples…", juce::Colours::white.withAlpha(0.5f));
        searchBox.onTextChange = [this]{ applyFilter(); };

        list.setRowHeight (24);
        list.setModel (&model);
    }

    ~SampleLibraryComponent() override { scanner.stopThread(2000); }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6);
        container.setBounds (r);

        auto inner = r.reduced (8);
        auto top   = inner.removeFromTop (28);
        addButton.setBounds (top.removeFromRight (28).reduced(2));
        searchBox.setBounds (top.removeFromLeft (inner.getWidth() - 36));
        list.setBounds (inner);
    }

    // Same API your DrumSamplerComponent already uses
    void addFile (const juce::File& fileToImport)
    {
        if (! fileToImport.existsAsFile()) return;

        auto destDir = DefaultSampleLibrary::installRoot().getChildFile ("UserImports");
        destDir.createDirectory();
        auto dest = destDir.getChildFile (fileToImport.getFileName());
        if (dest != fileToImport) (void) fileToImport.copyFileTo (dest);

        refreshList();
    }

private:
    // Dark rounded background
    struct DarkContainer : juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();
            g.setColour(juce::Colours::black.withAlpha(0.35f));
            g.fillRoundedRectangle(b, 8.0f);
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.drawRoundedRectangle(b, 8.0f, 1.0f);
        }
    } container;

    void openChooser()
    {
        auto startDir = DefaultSampleLibrary::installRoot();
        fileChooser = std::make_unique<juce::FileChooser>("Add sample(s)", startDir, "*.wav");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::canSelectMultipleItems
            | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& fc)
            {
                for (auto f : fc.getResults())
                    if (f.hasFileExtension("wav;WAV;aif;aiff;flac;mp3"))
                        addFile (f);

                refreshList();
                fileChooser.reset();
            });
    }

    void refreshList()
    {
        auto files = DefaultSampleLibrary::listAll();
        allFiles = files;
        model.setFiles (std::move(files));
        list.updateContent();
    }

    void applyFilter()
    {
        auto q = searchBox.getText().trim().toLowerCase();
        if (q.isEmpty())
        {
            model.setFiles (allFiles);
        }
        else
        {
            juce::Array<juce::File> filtered;
            for (auto& f : allFiles)
                if (f.getFileName().toLowerCase().contains (q))
                    filtered.add (f);
            model.setFiles (std::move(filtered));
        }
        list.updateContent();
    }


    juce::TimeSliceThread            scanner { "SampleLibrary Scanner" };
    std::unique_ptr<juce::FileChooser> fileChooser;

    SampleListModel model;
    juce::ListBox   list;
    juce::TextButton addButton { "+" };
    juce::TextEditor searchBox;

    juce::Array<juce::File> allFiles;
};


