#include "PluginManager.h"

//==============================================================================
// Local helpers

namespace
{
    /** Ensures the directory for the given file exists and returns it. */
    juce::File ensureDir (const juce::File& directory)
    {
        directory.createDirectory();
        return directory;
    }
}

//==============================================================================
// Construction

PluginManager::PluginManager (te::Edit& e, const Settings& s)
    : edit (e), settings (s)
{
    auto base = ensureDir (settings.appDataDir);
    knownListFile = base.getChildFile ("KnownPlugins.xml");
    blacklistFile = base.getChildFile ("PluginBlacklist.txt");
    deadMansFile  = base.getChildFile ("PluginScanDeadMans.txt");

    initFormats();
    loadKnownListFromDisk();
}

//==============================================================================
// Format initialisation

void PluginManager::initFormats()
{
   #if JUCE_PLUGINHOST_AU
    if (settings.scanAudioUnits)
        formatManager.addFormat (std::make_unique<juce::AudioUnitPluginFormat>());
   #endif

   #if JUCE_PLUGINHOST_VST3
    if (settings.scanVST3)
        formatManager.addFormat (std::make_unique<juce::VST3PluginFormat>());
   #endif
}

//==============================================================================
// Persistence

void PluginManager::saveKnownListToDisk() const
{
    if (auto xml = knownPlugins.createXml())
        xml->writeTo (knownListFile);

    // Persist blacklist as newline-separated paths.
    auto blacklisted = knownPlugins.getBlacklistedFiles();
    blacklistFile.replaceWithText (blacklisted.joinIntoString ("\n"));
}

void PluginManager::loadKnownListFromDisk()
{
    if (knownListFile.existsAsFile())
        if (auto xml = juce::XmlDocument::parse (knownListFile))
            knownPlugins.recreateFromXml (*xml);

    if (blacklistFile.existsAsFile())
    {
        juce::StringArray lines;
        blacklistFile.readLines (lines);

        for (auto& s : lines)
            knownPlugins.addToBlacklist (s);
    }
}

//==============================================================================
// Search paths

juce::FileSearchPath PluginManager::buildSearchPaths() const
{
    juce::StringArray pathStrings;

   #if JUCE_MAC
    if (settings.scanAudioUnits)
    {
        pathStrings.add ("/Library/Audio/Plug-Ins/Components");
        pathStrings.add ("~/Library/Audio/Plug-Ins/Components");
    }

    if (settings.scanVST3)
    {
        pathStrings.add ("/Library/Audio/Plug-Ins/VST3");
        pathStrings.add ("~/Library/Audio/Plug-Ins/VST3");
    }
   #elif JUCE_WINDOWS
    if (settings.scanVST3)
    {
        pathStrings.add ("C:\\Program Files\\Common Files\\VST3");
        pathStrings.add ("C:\\Program Files (x86)\\Common Files\\VST3");
    }
   #elif JUCE_LINUX
    if (settings.scanVST3)
    {
        pathStrings.add ("~/.vst3");
        pathStrings.add ("/usr/lib/vst3");
        pathStrings.add ("/usr/local/lib/vst3");
    }
   #endif

    return juce::FileSearchPath (pathStrings.joinIntoString (";"));
}

//==============================================================================
// Scanning control

void PluginManager::startNextFormatScan (bool reset)
{
    juce::ignoreUnused (reset);

    if (currentFormatIndex >= formatManager.getNumFormats())
    {
        scanner.reset();
        saveKnownListToDisk();
        stopTimer();
        return;
    }

    auto* format = formatManager.getFormat (currentFormatIndex++);

    scanner = std::make_unique<juce::PluginDirectoryScanner> (
        knownPlugins,
        *format,
        buildSearchPaths(),
        /* searchRecursively */ true,
        deadMansFile,
        /* allowAsyncInstantiation */ true);

    startTimerHz (30); // Pump scanner in timerCallback.
}

void PluginManager::startScanner (bool reset)
{
    currentFormatIndex = 0;
    startNextFormatScan (reset);

    if (scanner != nullptr)
        return;

    // Fallback path: only executes if startNextFormatScan did not create a scanner.
    auto searchPaths = buildSearchPaths();

    if (! scannerThread)
        scannerThread = std::make_unique<juce::TimeSliceThread> ("PluginScanThread");

    if (! scannerThread->isThreadRunning())
        scannerThread->startThread();

    if (formatManager.getNumFormats() > 0)
    {
        auto* format = formatManager.getFormat (0);

        scanner = std::make_unique<juce::PluginDirectoryScanner> (
            knownPlugins,
            *format,
            searchPaths,
            /* searchRecursively */ true,
            /* deadMansPedalFile */ deadMansFile,
            /* allowAsyncInstantiation */ true);

        startTimerHz (30);
    }
}

void PluginManager::timerCallback()
{
    if (! scanner)
    {
        stopTimer();
        return;
    }

    juce::String discovered;
    int pumped = 0;
    constexpr int maxPerTick = 8;

    // Pump a limited number of files per timer tick to keep the UI responsive.
    while (pumped++ < maxPerTick && scanner->scanNextFile (true, discovered))
    {
        // `discovered` is intentionally unused here; we only care about
        // progressing the scanner.
    }

    // Check if scanning is finished (using a non-advancing call).
    if (! scanner->scanNextFile (false, discovered))
    {
        stopTimer();
        scanner.reset();

        startNextFormatScan (/* reset */ false);
        saveKnownListToDisk();
    }
}

void PluginManager::pumpScannerUntilDone (juce::String progressPrefix)
{
    juce::ignoreUnused (progressPrefix);
    jassert (scanner != nullptr);

    juce::String pluginName;

    // Simple loop for blocking mode; async callers should not use this.
    while (scanner && scanner->scanNextFile (true, pluginName))
    {
        // `pluginName` and `progressPrefix` could be used for logging or
        // progress reporting if needed.
    }

    if (scanner)
    {
        scanner.reset();
        saveKnownListToDisk();
    }
}

//==============================================================================
// Public scanning API

void PluginManager::scanForPluginsAsync()
{
    startScanner (/* reset */ false);
}

void PluginManager::scanForPluginsBlocking()
{
    auto searchPaths = buildSearchPaths();

    for (int i = 0; i < formatManager.getNumFormats(); ++i)
    {
        auto* format = formatManager.getFormat (i);
        juce::String discovered;

        juce::PluginDirectoryScanner scannerInstance (
            knownPlugins,
            *format,
            searchPaths,
            /* searchRecursively */ true,
            /* deadMansPedalFile */ deadMansFile,
            /* allowAsyncInstantiation */ true);

        while (scannerInstance.scanNextFile (true, discovered))
        {
            // `discovered` could be used here for progress reporting.
        }
    }

    saveKnownListToDisk();
}

void PluginManager::rescanAsync (bool clearBlacklistFirst)
{
    if (clearBlacklistFirst)
    {
        knownPlugins.clearBlacklistedFiles();
        blacklistFile.deleteFile();
    }

    startScanner (/* reset */ true);
}

//==============================================================================
// Query helpers

juce::StringArray PluginManager::getAllPluginNames() const
{
    juce::StringArray names;
    const auto& types = knownPlugins.getTypes();

    for (int i = 0; i < types.size(); ++i)
        names.add (types.getReference (i).name);

    names.sort (true);
    return names;
}

juce::OwnedArray<juce::PluginDescription> PluginManager::getAllPluginDescriptions() const
{
    juce::OwnedArray<juce::PluginDescription> result;
    const auto& types = knownPlugins.getTypes();

    for (int i = 0; i < types.size(); ++i)
        result.add (new juce::PluginDescription (types.getReference (i)));

    return result;
}

juce::OwnedArray<juce::PluginDescription> PluginManager::getInstrumentDescriptions() const
{
    juce::OwnedArray<juce::PluginDescription> instruments;
    const auto& types = knownPlugins.getTypes();

    for (int i = 0; i < types.size(); ++i)
    {
        const auto& description = types.getReference (i);

        if (description.isInstrument)
            instruments.add (new juce::PluginDescription (description));
    }

    return instruments;
}

//==============================================================================
// Track helpers

te::Plugin::Ptr PluginManager::addExternalInstrumentToTrack (te::AudioTrack& track,
                                                             const juce::PluginDescription& desc,
                                                             int insertIndex)
{
    auto* descPtr = const_cast<juce::PluginDescription*> (&desc);

    auto plugin = track.edit.getPluginCache()
                       .createNewPlugin (te::ExternalPlugin::xmlTypeName, *descPtr);

    if (! plugin)
        return {};

    track.pluginList.insertPlugin (plugin, insertIndex, nullptr);
    auto* inserted = track.pluginList[insertIndex];

    if (auto* external = dynamic_cast<te::ExternalPlugin*> (inserted))
    {
        if (auto* instance = external->getAudioPluginInstance())
            return inserted;

        // If there is a load error, fall through and remove the plugin.
    }

    if (inserted != nullptr)
        inserted->deleteFromParent();

    return {};
}

te::Plugin::Ptr PluginManager::addExternalEffectToTrack (te::AudioTrack& track,
                                                         const juce::PluginDescription& desc,
                                                         int insertIndex)
{
    // Currently shares implementation with instrument insertion.
    return addExternalInstrumentToTrack (track, desc, insertIndex);
}
