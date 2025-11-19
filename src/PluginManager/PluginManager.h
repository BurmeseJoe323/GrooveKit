#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * Manages discovery and instantiation of external audio plugins (AUs, VST3, etc.)
 * for a given Edit. Handles scanning, known-plugin persistence, and helpers
 * for inserting plugins on tracks.
 */
class PluginManager : private juce::Timer
{
public:
    //==============================================================================
    /**
     * Configuration for plugin scanning and persistence.
     */
    struct Settings
    {
        juce::File appDataDir;     ///< Directory where known plugins, blacklist, etc. are stored.
        bool scanAudioUnits = true;
        bool scanVST3       = true;
    };

    //==============================================================================
    /**
     * Constructs a PluginManager for the given Edit using the provided Settings.
     *
     * This will initialise plugin formats and attempt to load a previously
     * saved known-plugin list and blacklist from disk.
     */
    explicit PluginManager (te::Edit& edit, const Settings& settings);

    //==============================================================================
    // Scanning API

    /**
     * Starts an asynchronous plugin scan using the current Settings.
     *
     * Call isScanRunning() to check progress, and rely on automatic saving
     * when scanning is complete (or call saveKnownListToDisk() explicitly).
     */
    void scanForPluginsAsync();

    /**
     * Performs a blocking scan over all supported plugin formats.
     *
     * This will not return until the scan has completed, and will then
     * write the known-plugin list and blacklist to disk.
     */
    void scanForPluginsBlocking();

    /**
     * Starts an asynchronous rescan.
     *
     * @param clearBlacklistFirst  If true, clears the blacklist and deletes
     *                             the blacklist file before starting the scan.
     */
    void rescanAsync (bool clearBlacklistFirst = false);

    //==============================================================================
    // State inspection / persistence

    /** Returns true while an asynchronous scan is in progress. */
    bool isScanRunning() const noexcept                 { return scanner != nullptr; }

    /** Returns the current known plugin list. */
    const juce::KnownPluginList& getKnownList() const noexcept { return knownPlugins; }

    /**
     * Loads the known-plugin list and blacklist from disk if present.
     *
     * Normally called during construction.
     */
    void loadKnownListFromDisk();

    /**
     * Writes the known-plugin list and blacklist to disk.
     */
    void saveKnownListToDisk() const;

    //==============================================================================
    // Query helpers

    /**
     * Returns a sorted list of all known plugin names.
     */
    juce::StringArray getAllPluginNames() const;

    /**
     * Returns a copy of all known plugin descriptions.
     */
    juce::OwnedArray<juce::PluginDescription> getAllPluginDescriptions() const;

    /**
     * Returns all known plugin descriptions that are instruments.
     */
    juce::OwnedArray<juce::PluginDescription> getInstrumentDescriptions() const;

    //==============================================================================
    // Track helpers

    /**
     * Inserts an external instrument plugin on the given AudioTrack.
     *
     * @param track        Track on which to insert the plugin.
     * @param desc         Plugin description retrieved from the known list.
     * @param insertIndex  Index at which the plugin should be added.
     *
     * @return A pointer to the inserted plugin on success, or a null pointer on failure.
     */
    tracktion::engine::Plugin::Ptr addExternalInstrumentToTrack (tracktion::engine::AudioTrack& track,
                                                                 const juce::PluginDescription& desc,
                                                                 int insertIndex);

    /**
     * Inserts an external effect plugin on the given AudioTrack.
     *
     * This currently delegates to addExternalInstrumentToTrack, but is
     * provided for semantic clarity and future differentiation.
     */
    te::Plugin::Ptr addExternalEffectToTrack (te::AudioTrack& track,
                                              const juce::PluginDescription& desc,
                                              int insertIndex);

private:
    //==============================================================================
    // Internal helpers

    /**
     * Registers plugin formats with the AudioPluginFormatManager
     * according to the current Settings.
     */
    void initFormats();

    /**
     * Builds platform-appropriate search paths for plugin scanning based on
     * the configured formats in Settings.
     */
    juce::FileSearchPath buildSearchPaths() const;

    /**
     * Starts the scanner over all formats, optionally resetting state.
     */
    void startScanner (bool reset);

    /**
     * Starts scanning the next plugin format in the AudioPluginFormatManager.
     *
     * Called internally when a format scan completes.
     */
    void startNextFormatScan (bool reset);

    /**
     * Timer callback used to pump the PluginDirectoryScanner asynchronously.
     */
    void timerCallback() override;

    /**
     * Pumps the scanner in a blocking loop until completion.
     *
     * Intended for blocking scan paths; async callers should not use this.
     */
    void pumpScannerUntilDone (juce::String progressPrefix);

    //==============================================================================
    // Data members

    te::Edit& edit;
    Settings  settings;

    juce::AudioPluginFormatManager        formatManager;
    juce::KnownPluginList                 knownPlugins;

    std::unique_ptr<juce::TimeSliceThread>       scannerThread;
    std::unique_ptr<juce::PluginDirectoryScanner> scanner;

    juce::File knownListFile;
    juce::File blacklistFile;
    juce::File deadMansFile;

    int currentFormatIndex = -1;
};
