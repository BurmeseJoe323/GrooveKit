// PluginManager.cpp
#include "PluginManager.h"
#include <juce_audio_processors/juce_audio_processors.h>

PluginManager::PluginManager()
{
    formats.addDefaultFormats();
}

static juce::FileSearchPath defaultSearchPathFor (juce::AudioPluginFormat& fmt)
{
    juce::StringArray dirs;

#if JUCE_MAC
    const auto name = fmt.getName();
    if (name == "VST3")
    {
        dirs.add ("/Library/Audio/Plug-Ins/VST3");
        dirs.add ("~/Library/Audio/Plug-Ins/VST3");
    }
    else if (name == "AudioUnit")
    {
        // AU discovery uses the system registry; returning components dirs is optional.
        dirs.add ("/Library/Audio/Plug-Ins/Components");
        dirs.add ("~/Library/Audio/Plug-Ins/Components");
    }
#elif JUCE_WINDOWS
    if (fmt.getName() == "VST3")
    {
        if (auto* cp   = std::getenv("CommonProgramFiles"))        dirs.add (juce::String(cp)   + "\\VST3");
        if (auto* cp86 = std::getenv("CommonProgramFiles(x86)"))   dirs.add (juce::String(cp86) + "\\VST3");
    }
#elif JUCE_LINUX
    if (fmt.getName() == "VST3")
    {
        dirs.add ("/usr/local/lib/vst3");
        dirs.add ("/usr/lib/vst3");
        dirs.add ("~/.vst3");
    }
#endif

    juce::FileSearchPath result;
    for (auto& d : dirs) result.add (juce::File (d)); // it's OK if a dir doesn't exist
    return result;
}



void PluginManager::scanDefaultPaths()
{
    auto deadMans = juce::File::getSpecialLocation (juce::File::tempDirectory)
                        .getChildFile ("GrooveKit_PluginScan_DeadMans");

    for (int i = 0; i < formats.getNumFormats(); ++i)
    {
        if (auto* fmt = formats.getFormat (i))
        {
            auto paths = defaultSearchPathFor (*fmt);

            DBG ("[Scan] Format: " << fmt->getName());
            DBG ("[Scan] Paths:  " << paths.toString());

            juce::PluginDirectoryScanner scanner (known, *fmt, paths,
                                                  /*recursive*/ true,
                                                  /*deadMans*/ deadMans,
                                                  /*allowAsync*/ false);

            juce::String nm;
            while (scanner.scanNextFile (true, nm))
                DBG ("[Scan] Probing: " << nm);
        }
    }
}

void PluginManager::clearCache()
{
    known.clear();
    known.clearBlacklistedFiles();
}

void PluginManager::rescanAllAndSave (juce::File cache, bool clearBlacklist)
{
    if (clearBlacklist) known.clearBlacklistedFiles();

    DBG("[Plugins] Rescanning default paths…");
    scanDefaultPaths();
    if (auto xml = known.createXml())
        xml->writeTo (cache);
}

bool PluginManager::scanOne (const juce::File& pluginBundle,
                             juce::OwnedArray<juce::PluginDescription>* outFound)
{
    using namespace juce;

    if (! pluginBundle.exists())
    {
        DBG ("[ScanOne] Not found: " << pluginBundle.getFullPathName());
        return false;
    }

    // Pick the format that claims this file
    AudioPluginFormat* chosen = nullptr;
    const String full = pluginBundle.getFullPathName();

    for (int i = 0; i < formats.getNumFormats(); ++i)
        if (auto* fmt = formats.getFormat(i))
            if (fmt->fileMightContainThisPluginType (full))   // pass String, not File
            { chosen = fmt; break; }

    if (chosen == nullptr)
    {
        DBG ("[ScanOne] No format claims: " << full);
        return false;
    }

    OwnedArray<PluginDescription> foundTmp;
    auto& sink = outFound ? *outFound : foundTmp;            // pass by reference

    // Some JUCE versions also have an overload with an error string.
    // If your compiler asks for it, add: String err; … scanAndAddFile(full, true, sink, *chosen, err);
    const bool ok = known.scanAndAddFile (full,
                                          /*dontRescanIfAlreadyInList*/ true,
                                          sink, *chosen);

    DBG ("[ScanOne] " << (ok ? "OK" : "FAILED")
         << ", found=" << sink.size()
         << " (" << full << ")");

    return ok && sink.size() > 0;
}



void PluginManager::save (juce::File storeTo)
{
    if (auto xml = known.createXml())
        xml->writeTo (storeTo);
}

void PluginManager::load (juce::File loadFrom)
{
    if (! loadFrom.existsAsFile())
        return;

    if (auto xml = juce::XmlDocument::parse (loadFrom))
        known.recreateFromXml (*xml);
}

std::optional<juce::PluginDescription>
PluginManager::findByName (const juce::String& name) const
{
    for (const auto& d : known.getTypes())   // d is a PluginDescription&
        if (d.name.equalsIgnoreCase (name))
            return d;                        // copy out

    return std::nullopt;
}
