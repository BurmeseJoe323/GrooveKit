#pragma once
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion::engine;

class PluginManager : private juce::Timer
{
public:

    struct Settings
    {
        juce::File appDataDir;
        bool scanAudioUnits = true;
        bool scanVST3 = true;
    };

    explicit PluginManager(te::Edit& e, const Settings&);

    void scanForPluginsAsync();
    void scanForPluginsBlocking();

    void rescanAsync(bool clearBlacklistFirst = false);

    bool isScanRunning() const noexcept { return scanner != nullptr; }
    const juce::KnownPluginList& getKnownList() const noexcept { return knownPlugins; }

    void loadKnownListFromDisk();
    void saveKnownListToDisk() const;

    juce::StringArray getAllPluginNames() const;
    juce::OwnedArray<juce::PluginDescription> getAllPluginDescriptions() const;
    juce::OwnedArray<juce::PluginDescription> getInstrumentDescriptions() const;

    tracktion::engine::Plugin::Ptr addExternalInstrumentToTrack (tracktion::engine::AudioTrack& track,
                                                             const juce::PluginDescription& desc,
                                                             int insertIndex);

    te::Plugin::Ptr addTALSynthToTrack(te::AudioTrack& track, int insertIndex = 0);

private:
    void initFormats();
    juce::FileSearchPath buildSearchPaths() const;
    void startScanner(bool reset);
    void startNextFormatScan(bool reset);
    void timerCallback() override;
    void pumpScannerUntilDone(juce::String progressPrefix);

    te::Edit& edit;

    Settings settings;

    juce::AudioPluginFormatManager formatManager;
    juce::KnownPluginList knownPlugins;

    std::unique_ptr<juce::TimeSliceThread> scannerThread;
    std::unique_ptr<juce::PluginDirectoryScanner> scanner;

    juce::File knownListFile;
    juce::File blacklistFile;
    juce::File deadMansFile;

    int currentFormatIndex = -1;

};