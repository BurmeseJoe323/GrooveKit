#include "PluginManager.h"

static juce::File ensureDir(const juce::File& f) { f.createDirectory(); return f; }

PluginManager::PluginManager(te::Edit& e, const Settings& s) : edit(e), settings(s)
{
    auto base = ensureDir(settings.appDataDir);
    knownListFile = base.getChildFile("KnownPlugins.xml");
    blacklistFile = base.getChildFile("PluginBlacklist.txt");
    deadMansFile  = base.getChildFile("PluginScanDeadMans.txt");

    initFormats();
    loadKnownListFromDisk();
}

void PluginManager::initFormats()
{
    #if JUCE_PLUGINHOST_AU
    if (settings.scanAudioUnits)
        formatManager.addFormat(new juce::AudioUnitPluginFormat());
    #endif

    #if JUCE_PLUGINHOST_VST3
    if (settings.scanVST3)
        formatManager.addFormat(new juce::VST3PluginFormat());
    #endif
}

void PluginManager::saveKnownListToDisk() const
{
    if (auto xml = knownPlugins.createXml())
        xml->writeTo(knownListFile);

    auto bl = knownPlugins.getBlacklistedFiles();          // <— use this
    blacklistFile.replaceWithText(bl.joinIntoString("\n"));
}

void PluginManager::loadKnownListFromDisk()
{
    if (knownListFile.existsAsFile())
        if (auto xml = juce::XmlDocument::parse(knownListFile))
            knownPlugins.recreateFromXml(*xml);

    if (blacklistFile.existsAsFile())
    {
        juce::StringArray lines;
        blacklistFile.readLines(lines);
        for (auto& s : lines)
            knownPlugins.addToBlacklist(s);
    }
}

juce::FileSearchPath PluginManager::buildSearchPaths() const
{
    juce::StringArray pathStrings;

    #if JUCE_MAC
    if (settings.scanAudioUnits) {
        pathStrings.add("/Library/Audio/Plug-Ins/Components");
        pathStrings.add("~/Library/Audio/Plug-Ins/Components");
    }
    if (settings.scanVST3) {
        pathStrings.add("/Library/Audio/Plug-Ins/VST3");
        pathStrings.add("~/Library/Audio/Plug-Ins/VST3");
    }
    #elif JUCE_WINDOWS
    if (settings.scanVST3) {
        pathStrings.add("C:\\Program Files\\Common Files\\VST3");
        pathStrings.add("C:\\Program Files (x86)\\Common Files\\VST3");
    }
    #elif JUCE_LINUX
    if (settings.scanVST3) {
        pathStrings.add("~/.vst3");
        pathStrings.add("/usr/lib/vst3");
        pathStrings.add("/usr/local/lib/vst3");
    }
    #endif

    return juce::FileSearchPath(pathStrings.joinIntoString(";"));
}

void PluginManager::startNextFormatScan(bool reset)
{
    if (currentFormatIndex >= formatManager.getNumFormats())
    {
        DBG("[PluginManager] Scan finished. Found: " + juce::String(knownPlugins.getNumTypes()));
        scanner.reset();
        saveKnownListToDisk();
        stopTimer();
        return;
    }

    auto* fmt = formatManager.getFormat(currentFormatIndex++);
    scanner = std::make_unique<juce::PluginDirectoryScanner>(
        knownPlugins, *fmt, buildSearchPaths(),
        /*searchRecursively*/ true, deadMansFile, /*allowAsync*/ true);

    DBG("[PluginManager] Scan started in " + fmt->getName()
        + " paths: " + buildSearchPaths().toString());

    startTimerHz(30); // keep pumping in timerCallback
}

void PluginManager::startScanner(bool reset)
{

    currentFormatIndex = 0;
    startNextFormatScan(reset);
    if (scanner != nullptr)
        return;

    auto searchPaths = buildSearchPaths();

    if (!scannerThread)
        scannerThread = std::make_unique<juce::TimeSliceThread>("PluginScanThread");
    if (!scannerThread->isThreadRunning())
        scannerThread->startThread();

    // Scan the first available format now (we’ll loop per-format after this works)
    if (formatManager.getNumFormats() > 0)
    {
        auto* fmt = formatManager.getFormat(0);

        // correct argument order and types:
        scanner = std::make_unique<juce::PluginDirectoryScanner>(
            knownPlugins,
            *fmt,
            searchPaths,
            /*searchRecursively*/ true,
            /*deadMansPedalFile*/ deadMansFile,
            /*allowAsyncInstantiation*/ true
        );

        DBG("[PluginManager] Scan started in " + fmt->getName()
            + " paths: " + searchPaths.toString());
        startTimerHz(30);
    }
}

void PluginManager::timerCallback()
{
    if (!scanner)
    {
        stopTimer();
        return;
    }

    juce::String discovered;
    // pump a few files per tick to keep UI snappy
    int pumped = 0;
    const int maxPerTick = 8;

    while (pumped++ < maxPerTick && scanner->scanNextFile(true, discovered))
    {
        if (discovered.isNotEmpty())
            DBG("[PluginScan] " + discovered);
    }

    // finished?
    if (!scanner->scanNextFile(false, discovered)) // peek without advancing
    {
        stopTimer();
        DBG("[PluginManager] Scan finished. Found: "
            + juce::String(knownPlugins.getNumTypes()) + " plugins.");

        scanner.reset();
        startNextFormatScan(/*reset*/ false);
        saveKnownListToDisk();
    }
}

void PluginManager::pumpScannerUntilDone (juce::String progressPrefix)
{
    jassert(scanner != nullptr);

    // Simple loop for blocking mode; async callers shouldn’t use this.
    juce::String pluginName;
    while (scanner && scanner->scanNextFile(true, pluginName))
    {
        DBG(progressPrefix + pluginName);
    }

    if (scanner)
    {
        DBG("[PluginManager] Scan finished. Found: "
            + juce::String (knownPlugins.getNumTypes()) + " plugins.");
        scanner.reset();
        saveKnownListToDisk();
    }
}

void PluginManager::scanForPluginsAsync()
{
    startScanner(/*reset*/ false);
    // Fire-and-forget: you can poll isScanRunning() or hook a timer elsewhere to
    // detect completion and then call saveKnownListToDisk().
    // For safety, auto-save when the scan object drops in the message loop.
}

void PluginManager::scanForPluginsBlocking()
{
    auto searchPaths = buildSearchPaths();

    for (int i = 0; i < formatManager.getNumFormats(); ++i)
    {
        auto* fmt = formatManager.getFormat(i);
        juce::String discovered;

        juce::PluginDirectoryScanner s(
            knownPlugins,
            *fmt,
            searchPaths,
            /*searchRecursively*/ true,
            /*deadMans*/ deadMansFile,
            /*allowAsync*/ true
        );

        while (s.scanNextFile(true, discovered))
            DBG("[PluginScan] " + fmt->getName() + ": " + discovered);
    }

    DBG("[PluginManager] Scan finished. Found: " + juce::String(knownPlugins.getNumTypes()));
    saveKnownListToDisk();
}

void PluginManager::rescanAsync(bool clearBlacklistFirst)
{
    if (clearBlacklistFirst)
    {
        knownPlugins.clearBlacklistedFiles();   // <—
        blacklistFile.deleteFile();
    }
    startScanner(/*reset*/ true);
}

juce::StringArray PluginManager::getAllPluginNames() const
{
    juce::StringArray names;
    const auto& types = knownPlugins.getTypes();

    for (int i = 0; i < types.size(); ++i)
        names.add(types.getReference(i).name);

    names.sort(true);
    return names;
}

juce::OwnedArray<juce::PluginDescription> PluginManager::getAllPluginDescriptions() const
{
    juce::OwnedArray<juce::PluginDescription> out;
    const auto& types = knownPlugins.getTypes();
    for (int i = 0; i < types.size(); ++i)
        out.add(new juce::PluginDescription(types.getReference(i)));
    return out;
}

te::Plugin::Ptr PluginManager::addTALSynthToTrack (te::AudioTrack& track, int insertIndex)
{
    DBG("[PluginManager] addTALSynthToTrack() called for track: " + track.getName());

    const juce::PluginDescription* dVST3 = nullptr;
    const juce::PluginDescription* dAU   = nullptr;

    // Find TAL-NoiseMaker descriptors we can try
    const auto& types = knownPlugins.getTypes();
    for (int i = 0; i < types.size(); ++i)
    {
        const auto& d = types.getReference(i);
        if (!d.isInstrument) continue;
        if (!d.name.containsIgnoreCase("TAL-NoiseMaker")) continue;

        if (d.pluginFormatName == "VST3") dVST3 = &d;
        else if (d.pluginFormatName.startsWith("AudioUnit")) dAU = &d;
    }

    if (!dVST3 && !dAU)
    {
        DBG("[PluginManager] TAL-NoiseMaker not found among scanned instruments.");
        return {};
    }

    auto tryCreateFromDesc = [&] (const juce::PluginDescription& desc) -> te::Plugin::Ptr
    {
        DBG("[PluginManager] Trying: " + desc.name + " [" + desc.pluginFormatName + "] path=" + desc.fileOrIdentifier);
        auto* descPtr = const_cast<juce::PluginDescription*>(&desc);
        return track.edit.getPluginCache()
                         .createNewPlugin(te::ExternalPlugin::xmlTypeName, *descPtr);
    };

    auto insertAndVerify = [&] (const juce::PluginDescription* desc) -> te::Plugin::Ptr
    {
        if (!desc) return {};

        // Create ExternalPlugin from description
        auto plugin = tryCreateFromDesc(*desc);
        if (!plugin)
        {
            DBG("[PluginManager] createNewPlugin returned nullptr for " << desc->name);
            return {};
        }

        // Insert into track
        track.pluginList.insertPlugin(plugin, insertIndex, nullptr);  // modifies ValueTree, instantiates list object
        auto* inserted = track.pluginList[insertIndex];               // fetch the just-inserted plugin
        if (!inserted) return {};

        DBG("[PluginManager] Inserted '" + inserted->getName()
            + "' at index " + juce::String(insertIndex));

        // Sanity-check the ExternalPlugin instance & load error
        if (auto* ext = dynamic_cast<te::ExternalPlugin*>(inserted))
        {
            auto* pi = ext->getAudioPluginInstance();                // non-null when instance loaded :contentReference[oaicite:2]{index=2}
            DBG("[Ext] has instance: " << (pi ? "true" : "false"));

            if (pi)
            {
                DBG("[Ext] name=" << pi->getName()
                    << " acceptsMidi=" << (pi->acceptsMidi() ? "true":"false")
                    << " isInstrument=" << (pi->getPluginDescription().isInstrument ? "true":"false")
                    << " ins=" << pi->getTotalNumInputChannels()
                    << " outs=" << pi->getTotalNumOutputChannels());
                return inserted;                                     // success
            }

            // Failed: log explicit load error and remove the plugin cleanly
            DBG("[Ext] FAIL '" << desc->name
                << "' [" << desc->pluginFormatName
                << "] error='" << ext->getLoadError() << "'");       // returns message if not loaded :contentReference[oaicite:3]{index=3}

            inserted->deleteFromParent();                             // correct removal on this TE branch :contentReference[oaicite:4]{index=4}
            return {};
        }

        // Not an ExternalPlugin? Remove and abort.
        inserted->deleteFromParent();
        return {};
    };

    // Try VST3 first, then AU fallback
    te::Plugin::Ptr ok;
    if (!ok && dVST3) ok = insertAndVerify(dVST3);
    if (!ok && dAU)   ok = insertAndVerify(dAU);

    if (!ok)
    {
        DBG("[PluginManager] TAL-NoiseMaker failed to instantiate in both VST3 and AU.");
        return {};
    }

    // Log final chain
    for (auto* p : track.pluginList.getPlugins())
        if (p) DBG("[PluginManager] Track now has plugin: " + p->getName());  // enumerate chain :contentReference[oaicite:5]{index=5}

    DBG("[PluginManager] Successfully inserted TAL-NoiseMaker on '" + track.getName() + "'");
    return ok;
}


