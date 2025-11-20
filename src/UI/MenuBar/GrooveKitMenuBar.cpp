// Created by Claude Code on 2025-11-15.
// GrooveKitMenuBar - Shared menu bar component implementation

#include "GrooveKitMenuBar.h"
#include "../Settings/SettingsDialog.h"

#include <TrackView/ExportOverlayComponent.h>

GrooveKitMenuBar::GrooveKitMenuBar(AppEngine& engine)
{
    appEngine = std::shared_ptr<AppEngine>(&engine, [](AppEngine*) {});

    #if JUCE_MAC
    // Use native macOS global menu bar
    juce::MenuBarModel::setMacMainMenu(this);
    #else
    // Create inline menu bar for Windows/Linux
    menuBarComponent = std::make_unique<juce::MenuBarComponent>(this);
    addAndMakeVisible(menuBarComponent.get());
    #endif
}

GrooveKitMenuBar::~GrooveKitMenuBar()
{
    #if JUCE_MAC
    // Clear the native macOS menu bar to avoid assertions during shutdown
    juce::MenuBarModel::setMacMainMenu(nullptr);
    #endif
}

void GrooveKitMenuBar::resized()
{
    #if !JUCE_MAC
    if (menuBarComponent)
        menuBarComponent->setBounds(getLocalBounds());
    #endif
}

juce::StringArray GrooveKitMenuBar::getMenuBarNames()
{
    // Show Track menu in both views (Written by Claude Code)
    return { "File", "View", "Track", "Help" };
}

juce::PopupMenu GrooveKitMenuBar::getMenuForIndex(const int topLevelMenuIndex, const juce::String&)
{
    juce::PopupMenu menu;
    enum MenuIDs
    {
        SwitchToTrackEdit = 1001, // (Written by Claude Code)
        OpenMixer = 1002,
        ShowPreferences = 1003, // (Written by Claude Code)
        NewEdit = 2001,
        OpenEdit = 2002,
        SaveEdit = 2003,
        SaveEditAs = 2004,
        ExportAudio = 2005,
        NewInstrumentTrack = 3001,
        NewDrumTrack = 3002
    };

    if (topLevelMenuIndex == 0) // File
    {
        menu.addItem(NewEdit, "New Edit");
        menu.addItem(OpenEdit, "Open Edit...");
        menu.addSeparator();
        menu.addItem(SaveEdit, "Save Edit");
        menu.addItem(SaveEditAs, "Save Edit As...");
        menu.addItem (ExportAudio, "Export Audio");
        menu.addSeparator();
        menu.addItem(ShowPreferences, "Preferences..."); // (Written by Claude Code)
    }
    else if (topLevelMenuIndex == 1) // View (Written by Claude Code)
    {
        // Show both views with checkmark on current view
        menu.addItem(SwitchToTrackEdit, "Track Edit", true, currentViewMode == ViewMode::TrackEdit);
        menu.addItem(OpenMixer, "Mix View", true, currentViewMode == ViewMode::Mix);
    }
    else if (topLevelMenuIndex == 2) // Track (Written by Claude Code)
    {
        menu.addItem(NewInstrumentTrack, "New Instrument Track");
        menu.addItem(NewDrumTrack, "New Drum Track");
    }
    // topLevelMenuIndex == 3 or 2 (depending on view mode) is Help - empty for now

    return menu;
}

void GrooveKitMenuBar::menuItemSelected(const int menuItemID, int)
{
    enum MenuIDs
    {
        SwitchToTrackEdit = 1001, // (Written by Claude Code)
        OpenMixer = 1002,
        ShowPreferences = 1003, // (Written by Claude Code)
        NewEdit = 2001,
        OpenEdit = 2002,
        SaveEdit = 2003,
        SaveEditAs = 2004,
        ExportAudio = 2005,
        NewInstrumentTrack = 3001,
        NewDrumTrack = 3002
    };

    switch (menuItemID)
    {
        case NewInstrumentTrack:
            if (onNewInstrumentTrack)
                onNewInstrumentTrack();
            break;
        case NewDrumTrack:
            if (onNewDrumTrack)
                onNewDrumTrack();
            break;
        case SwitchToTrackEdit: // (Written by Claude Code)
            if (onSwitchToTrackEdit)
                onSwitchToTrackEdit();
            break;
        case OpenMixer:
            if (onSwitchToMix)
                onSwitchToMix();
            break;
        case ShowPreferences: // (Written by Claude Code)
            showPreferences();
            break;
        case NewEdit:
            showNewEditMenu();
            break;
        case OpenEdit:
            showOpenEditMenu();
            break;
        case SaveEdit:
            appEngine->saveEdit();
            break;
        case SaveEditAs:
            appEngine->saveEditAsAsync();
            break;
        case ExportAudio:
            exportAudio();
        break;
        default:
            break;
    }
}

void GrooveKitMenuBar::setViewMode(ViewMode mode)
{
    currentViewMode = mode;
    menuItemsChanged();  // Notify JUCE to rebuild menus
}

void GrooveKitMenuBar::showPreferences() const
{
    // Created by Claude Code on 2025-11-18.
    auto* settingsComponent = new SettingsDialog(*appEngine);
    settingsComponent->setSize(600, 400);

    juce::DialogWindow::LaunchOptions opts;
    opts.content.setOwned(settingsComponent);
    opts.dialogTitle = "Preferences";
    opts.resizable = false;
    opts.useNativeTitleBar = true;
    opts.launchAsync();
}

void GrooveKitMenuBar::showNewEditMenu() const
{
    if (appEngine->isDirty())
    {
        const auto opts = juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::WarningIcon)
            .withTitle("Save changes?")
            .withMessage("You have unsaved changes.")
            .withButton("Save")
            .withButton("Discard")
            .withButton("Cancel");

        juce::AlertWindow::showAsync(opts,
            [this](const int r) {
                if (r == 1)
                {
                    // Save
                    const bool hasPath =
                        appEngine->getCurrentEditFile().getFullPathName().isNotEmpty();
                    if (hasPath)
                    {
                        if (appEngine->saveEdit())
                            appEngine->newUntitledEdit();
                    }
                    else
                    {
                        appEngine->saveEditAsAsync([this](const bool ok) {
                            if (ok)
                                appEngine->newUntitledEdit();
                        });
                    }
                }
                else if (r == 2)
                {
                    // Discard
                    appEngine->newUntitledEdit();
                }
            });
    }
    else
    {
        appEngine->newUntitledEdit();
    }
}

void GrooveKitMenuBar::showOpenEditMenu() const
{
    if (!appEngine->isDirty())
    {
        appEngine->openEditAsync();
        return;
    }

    const auto opts = juce::MessageBoxOptions()
        .withIconType(juce::MessageBoxIconType::WarningIcon)
        .withTitle("Save changes?")
        .withMessage("You have unsaved changes.")
        .withButton("Save")
        .withButton("Discard")
        .withButton("Cancel");

    juce::AlertWindow::showAsync(opts,
        [this](const int result) {
            if (result == 1) // Save
            {
                if (appEngine->getCurrentEditFile().getFullPathName().isNotEmpty())
                {
                    if (appEngine->saveEdit())
                        appEngine->openEditAsync();
                }
                else
                {
                    appEngine->saveEditAsAsync([this](const bool ok) {
                        if (ok)
                            appEngine->openEditAsync();
                    });
                }
            }
            else if (result == 2) // Discard
            {
                appEngine->openEditAsync();
            }
        });
}
void GrooveKitMenuBar::exportAudio()
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Export audio",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory),
        "*.wav");

    chooser->launchAsync (juce::FileBrowserComponent::saveMode
                          | juce::FileBrowserComponent::canSelectFiles,
                          [this, chooser] (const juce::FileChooser& fc)
    {
        DBG ("[TrackEditView] ExportAudio chooser callback hit");

        auto file = fc.getResult();

        if (file == juce::File())
            return; // user cancelled

        file = file.withFileExtension (".wav");

        // Create and show full-screen overlay
        exportOverlay = std::make_unique<ExportOverlayComponent>();
        exportOverlay->setBounds (getLocalBounds());
        addAndMakeVisible (exportOverlay.get());
        exportOverlay->toFront (true);
        repaint();

        // Do the export
        const bool ok = appEngine->exportAudio (file);

        // Remove overlay
        if (exportOverlay != nullptr)
        {
            removeChildComponent (exportOverlay.get());
            exportOverlay.reset();
        }

        // Show result
        if (! ok)
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                    "Export failed",
                                                    "There was a problem rendering the audio.");
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon,
                                                    "Export complete",
                                                    "audio exported to:\n" + file.getFullPathName());
        }
    });
}
