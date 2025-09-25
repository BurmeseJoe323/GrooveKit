// PluginManager.h
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class PluginManager
{
public:
    PluginManager();

    void scanDefaultPaths();
    void clearCache();
    void rescanAllAndSave (juce::File cacheFile, bool clearBlacklist = true);
    bool scanOne (const juce::File& pluginBundle, juce::OwnedArray<juce::PluginDescription>* outFound = nullptr);

    void save (juce::File storeTo);
    void load (juce::File loadFrom);

    juce::KnownPluginList& getKnownList()          { return known; }
    juce::AudioPluginFormatManager& getFormats()   { return formats; }

    std::optional<juce::PluginDescription> findByName (const juce::String& name) const;

private:
    juce::AudioPluginFormatManager formats;
    juce::KnownPluginList known;
    std::unique_ptr<juce::PropertiesFile> props;
};
