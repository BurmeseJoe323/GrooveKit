#pragma once
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion::engine;

class PluginManager {
public:
    explicit PluginManager(te::Edit& e) : edit(e) {}

    // Create a FourOSC instance (built-in) and return it
    te::Plugin::Ptr createFourOSC();

    // Insert FourOSC at index 0 on a MIDI track; returns the plugin
    te::Plugin::Ptr addFourOSCToTrack(te::AudioTrack& track);

private:
    te::Edit& edit;
};