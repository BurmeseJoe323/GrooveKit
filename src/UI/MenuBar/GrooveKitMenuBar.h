// Created by Claude Code on 2025-11-15.
// GrooveKitMenuBar - Shared menu bar component for TrackEditView and MixView

#pragma once

#include "../../AppEngine/AppEngine.h"
#include <TrackView/ExportOverlayComponent.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace te = tracktion::engine;
namespace t = tracktion;

class AppEngine;

/**
 * Menu bar component that appears in both TrackEditView and MixView.
 * Handles File, View, Track, and Help menus. (Written by Claude Code)
 */
class GrooveKitMenuBar final : public juce::Component, public juce::MenuBarModel
{
public:
    enum class ViewMode
    {
        TrackEdit,  // Track Edit view mode (Written by Claude Code)
        Mix         // Mix view mode
    };

    explicit GrooveKitMenuBar(AppEngine& engine);
    ~GrooveKitMenuBar() override;

    void resized() override;

    // MenuBarModel overrides
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    /**
     * Set the current view mode to control which menu items are visible.
     */
    void setViewMode(ViewMode mode);

    /**
     * Callbacks for view-specific actions.
     */
    std::function<void()> onSwitchToMix;
    std::function<void()> onSwitchToTrackEdit; // (Written by Claude Code)
    std::function<void()> onNewInstrumentTrack;
    std::function<void()> onNewDrumTrack;

private:
    void showPreferences() const; // (Written by Claude Code)
    void showNewEditMenu() const;
    void showOpenEditMenu() const;
    void exportAudio();

    std::shared_ptr<AppEngine> appEngine;
    std::unique_ptr<ExportOverlayComponent> exportOverlay;
    ViewMode currentViewMode = ViewMode::TrackEdit;

    #if !JUCE_MAC
    std::unique_ptr<juce::MenuBarComponent> menuBarComponent;
    #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveKitMenuBar)
};
