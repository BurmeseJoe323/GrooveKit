#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * @brief Manages discovery and instantiation of external audio plugins.
 *
 * PluginManager wraps Tracktion Engine's plugin scanning and known-list
 * functionality for a given Edit. It:
 *  - Scans AU/VST3 (depending on settings and platform).
 *  - Persists the known-plugin list, blacklist, and dead-man's file.
 *  - Provides helpers for inserting external instrument/effect plugins
 *    onto AudioTracks.
 */
class PluginManager : private juce::Timer
{
public:
    //==============================================================================
    /**
     * @brief Configuration for plugin scanning and persistence.
     *
     * Controls which plugin formats are scanned and where plugin metadata
     * and blacklist files are stored on disk.
     */
    struct Settings
    {
        juce::File appDataDir; ///< Directory where known plugins, blacklist, etc. are stored.
        bool scanAudioUnits = true; ///< Whether to scan AudioUnit plugins (macOS only).
        bool scanVST3       = true; ///< Whether to scan VST3 plugins.
    };

    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs a PluginManager for the given Edit using the provided settings.
     *
     * This will initialise plugin formats and attempt to load a previously
     * saved known-plugin list and blacklist from disk.
     *
     * @param edit      Tracktion Edit this PluginManager is associated with.
     * @param settings  Scanning and persistence settings.
     */
    explicit PluginManager (te::Edit& edit, const Settings& settings);

    //==============================================================================
    // Scanning API

    /**
     * @brief Starts an asynchronous plugin scan using the current settings.
     *
     * Use isScanRunning() to check progress. When scanning completes, the
     * known-plugin list and blacklist will be written to disk automatically
     * (or you can call saveKnownListToDisk() explicitly).
     */
    void scanForPluginsAsync();

    /**
     * @brief Performs a blocking scan over all supported plugin formats.
     *
     * This call does not return until the scan has completed. On completion,
     * the known-plugin list and blacklist are written to disk.
     */
    void scanForPluginsBlocking();

    /**
     * @brief Starts an asynchronous rescan of plugins.
     *
     * @param clearBlacklistFirst  If true, clears the blacklist in memory and
     *                             deletes the on-disk blacklist file before scanning.
     */
    void rescanAsync (bool clearBlacklistFirst = false);

    //==============================================================================
    // State inspection / persistence

    /** @brief Returns true while an asynchronous scan is in progress. */
    bool isScanRunning() const noexcept                 { return scanner != nullptr; }

    /** @brief Returns the current known plugin list. */
    const juce::KnownPluginList& getKnownList() const noexcept { return knownPlugins; }

    /**
     * @brief Loads the known-plugin list and blacklist from disk if present.
     *
     * Normally called during construction. Safe to call again to re-sync
     * from disk.
     */
    void loadKnownListFromDisk();

    /**
     * @brief Writes the known-plugin list and blacklist to disk.
     */
    void saveKnownListToDisk() const;

    //==============================================================================
    // Query helpers

    /**
     * @brief Returns a sorted list of all known plugin names.
     */
    juce::StringArray getAllPluginNames() const;

    /**
     * @brief Returns a copy of all known plugin descriptions.
     */
    juce::OwnedArray<juce::PluginDescription> getAllPluginDescriptions() const;

    /**
     * @brief Returns all known plugin descriptions that are instruments.
     */
    juce::OwnedArray<juce::PluginDescription> getInstrumentDescriptions() const;

    //==============================================================================
    // Track helpers

    /**
     * @brief Inserts an external instrument plugin on the given AudioTrack.
     *
     * @param track        Track on which to insert the plugin.
     * @param desc         Plugin description retrieved from the known list.
     * @param insertIndex  Index at which the plugin should be added.
     *
     * @return A pointer to the inserted plugin on success, or an empty Ptr on failure.
     */
    te::Plugin::Ptr addExternalInstrumentToTrack (te::AudioTrack& track,
                                                  const juce::PluginDescription& desc,
                                                  int insertIndex);

    /**
     * @brief Inserts an external effect plugin on the given AudioTrack.
     *
     * Currently shares the same implementation as addExternalInstrumentToTrack,
     * but exists for semantic clarity and future differentiation.
     *
     * @param track        Track on which to insert the plugin.
     * @param desc         Plugin description retrieved from the known list.
     * @param insertIndex  Index at which the plugin should be added.
     *
     * @return A pointer to the inserted plugin on success, or an empty Ptr on failure.
     */
    te::Plugin::Ptr addExternalEffectToTrack (te::AudioTrack& track,
                                              const juce::PluginDescription& desc,
                                              int insertIndex);

private:
    //==============================================================================
    // Internal helpers

    /**
     * @brief Registers plugin formats with the AudioPluginFormatManager.
     *
     * Respects the current settings (e.g. scanAudioUnits, scanVST3).
     */
    void initFormats();

    /**
     * @brief Builds platform-appropriate search paths for plugin scanning.
     *
     * The paths depend on the configured formats and the target platform.
     */
    juce::FileSearchPath buildSearchPaths() const;

    /**
     * @brief Starts the scanner over all formats.
     *
     * @param reset Whether to reset scanning state (e.g. when rescanning).
     */
    void startScanner (bool reset);

    /**
     * @brief Starts scanning the next plugin format in the manager.
     *
     * Called internally when a format scan completes.
     *
     * @param reset Whether this is part of a reset/rescan operation.
     */
    void startNextFormatScan (bool reset);

    /**
     * @brief Timer callback used to pump the PluginDirectoryScanner asynchronously.
     */
    void timerCallback() override;

    /**
     * @brief Pumps the scanner in a blocking loop until completion.
     *
     * Intended for blocking scan paths; async callers should not use this.
     *
     * @param progressPrefix Optional string to prefix any logging/progress output.
     */
    void pumpScannerUntilDone (juce::String progressPrefix);

    //==============================================================================
    // Member variables

    te::Edit& edit;   ///< Edit this PluginManager belongs to.
    Settings  settings; ///< Scanning and persistence configuration.

    juce::AudioPluginFormatManager        formatManager; ///< Registered plugin formats.
    juce::KnownPluginList                 knownPlugins;  ///< In-memory known plugin list.

    std::unique_ptr<juce::TimeSliceThread>        scannerThread;  ///< Thread used for async scanning.
    std::unique_ptr<juce::PluginDirectoryScanner> scanner;        ///< Active directory scanner.

    juce::File knownListFile;   ///< XML file storing the known-plugin list.
    juce::File blacklistFile;   ///< Text file storing blacklisted plugin paths.
    juce::File deadMansFile;    ///< Dead-man's file used by JUCE during scanning.

    int currentFormatIndex = -1; ///< Index of the format currently being scanned.
};
