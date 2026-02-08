#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "DrumSamplerEngine/DefaultSampleLibrary.h"
#include <memory>

/**
 * @brief A single row in the sample list.
 *
 * Responsible for drawing the sample name and initiating a drag operation
 * when the user drags the row. The drag description is the absolute path
 * to the sample file, so drop targets can load it easily.
 */
class SampleRow : public juce::Component
{
public:
    explicit SampleRow (const juce::File& f) : file (f) {}

    //==============================================================================
    /** Draws the row background and the file name. */
    void paint (juce::Graphics& g) override
    {
        g.fillAll (isMouseOver()
                       ? juce::Colours::black.withAlpha (0.2f)
                       : juce::Colours::transparentBlack);

        g.setColour (juce::Colours::white);
        g.setFont   (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));

        g.drawFittedText (file.getFileNameWithoutExtension(),
                          getLocalBounds().reduced (8, 2),
                          juce::Justification::centredLeft,
                          1);

        g.setColour (juce::Colours::white.withAlpha (0.3f));
        g.drawLine  (0.0f,
                     (float) getHeight() - 0.5f,
                     (float) getWidth(),
                     (float) getHeight() - 0.5f);
    }

    //==============================================================================
    /**
     * @brief Starts a drag operation the first time the mouse is dragged.
     *
     * The drag description is the file's full path as a string, so drop
     * targets can treat it as a juce::File.
     */
    void mouseDrag (const juce::MouseEvent&) override
    {
        if (! hasStartedDrag)
            if (auto* dnd = juce::DragAndDropContainer::findParentDragContainerFor (this))
            {
                hasStartedDrag = true;
                // description = absolute path
                dnd->startDragging (file.getFullPathName(), this);
            }
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        hasStartedDrag = false;
    }

    //==============================================================================
    /** Returns the file represented by this row. */
    const juce::File& getFile() const { return file; }

private:
    juce::File file;
    bool hasStartedDrag = false;
};

/**
 * @brief ListBoxModel used by SampleLibraryComponent to display SampleRow items.
 *
 * It holds an array of files and hands out SampleRow components as needed.
 */
class SampleListModel : public juce::ListBoxModel
{
public:
    /** Replaces the current list of files. */
    void setFiles (juce::Array<juce::File> newFiles)
    {
        files.swapWith (newFiles);
    }

    //==============================================================================
    int getNumRows() override
    {
        return files.size();
    }

    /** ListBox paints are delegated entirely to SampleRow, so this does nothing. */
    void paintListBoxItem (int, juce::Graphics&, int, int, bool) override {}

    /**
     * @brief Returns or creates the row component for a given index.
     *
     * Reuses the existing component where possible; otherwise creates a new SampleRow.
     */
    juce::Component* refreshComponentForRow (int row,
                                             bool,
                                             juce::Component* existing) override
    {
        if (! juce::isPositiveAndBelow (row, files.size()))
            return nullptr;

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

/**
 * @brief UI component showing the installed sample library with search + drag support.
 *
 * Features:
 *  - Ensures the default library is installed on construction.
 *  - Displays all discovered samples in a ListBox.
 *  - Provides a search box to filter by filename.
 *  - Allows importing user samples via FileChooser (saved under UserImports).
 *  - Supports dragging rows (SampleRow) into other components (e.g., drum pads).
 */
class SampleLibraryComponent : public juce::Component
{
public:
    SampleLibraryComponent()
    {
        // Make sure built-in samples are on disk before listing them.
        DefaultSampleLibrary::ensureInstalled();

        refreshList();

        addAndMakeVisible (container);
        addAndMakeVisible (addButton);
        addAndMakeVisible (searchBox);
        addAndMakeVisible (list);

        addButton.setButtonText ("+");
        addButton.setTooltip ("Add sample(s) to library");
        addButton.onClick = [this] { openChooser(); };

        searchBox.setTextToShowWhenEmpty ("Filter samples",
                                          juce::Colours::white.withAlpha (0.5f));
        searchBox.onTextChange = [this] { applyFilter(); };

        list.setRowHeight (24);
        list.setModel (&model);
    }

    ~SampleLibraryComponent() override
    {
        scanner.stopThread (2000);
    }

    //==============================================================================
    /** Layouts the container, header row (button + search), and the list. */
    void resized() override
    {
        auto r = getLocalBounds().reduced (6);
        container.setBounds (r);

        auto inner = r.reduced (8);
        auto top   = inner.removeFromTop (28);

        addButton.setBounds (top.removeFromRight (28).reduced (2));
        searchBox.setBounds (top.removeFromLeft (inner.getWidth() - 36));

        list.setBounds (inner);
    }

    /**
     * @brief Adds a sample file to the library's "UserImports" folder and refreshes.
     *
     * If the file is outside the library root, it is copied into the UserImports
     * folder inside DefaultSampleLibrary::installRoot().
     */
    void addFile (const juce::File& fileToImport)
    {
        if (! fileToImport.existsAsFile())
            return;

        auto destDir = DefaultSampleLibrary::installRoot().getChildFile ("UserImports");
        destDir.createDirectory();

        auto dest = destDir.getChildFile (fileToImport.getFileName());
        if (dest != fileToImport)
            (void) fileToImport.copyFileTo (dest);

        refreshList();
    }

private:
    /**
     * @brief Simple rounded-rectangle background container for the library UI.
     */
    struct DarkContainer : juce::Component
    {
        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();

            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.fillRoundedRectangle (b, 8.0f);

            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.drawRoundedRectangle (b, 8.0f, 1.0f);
        }
    } container;

    //==============================================================================
    /** Opens a FileChooser to let the user add one or more new samples. */
    void openChooser()
    {
        auto startDir = DefaultSampleLibrary::installRoot();

        fileChooser = std::make_unique<juce::FileChooser> (
            "Add sample(s)", startDir, "*.wav");

        fileChooser->launchAsync (
            juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::canSelectMultipleItems
            | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& fc)
            {
                for (auto f : fc.getResults())
                    if (f.hasFileExtension ("wav;WAV;aif;aiff;flac;mp3"))
                        addFile (f);

                refreshList();
                fileChooser.reset();
            });
    }

    /** Refreshes the list from DefaultSampleLibrary and resets the model contents. */
    void refreshList()
    {
        auto files = DefaultSampleLibrary::listAll();
        allFiles   = files;

        model.setFiles (std::move (files));
        list.updateContent();
    }

    /** Applies the current search text to filter the visible samples. */
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

            model.setFiles (std::move (filtered));
        }

        list.updateContent();
    }

    //==============================================================================
    juce::TimeSliceThread              scanner { "SampleLibrary Scanner" };
    std::unique_ptr<juce::FileChooser> fileChooser;

    SampleListModel model;
    juce::ListBox   list;
    juce::TextButton addButton { "+" };
    juce::TextEditor searchBox;

    juce::Array<juce::File> allFiles;
};