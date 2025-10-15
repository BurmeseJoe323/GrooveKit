#include "PluginManager.h"

// te::Plugin::Ptr PluginManager::createFourOSC()
// {
//     //auto& pm = edit.getPluginManager();
//     // FourOSC is part of tracktion_plugins; xml type is registered by the module
//     //return pm.createNewPlugin (te::FourOscPlugin::xmlTypeName, {});
// }

te::Plugin::Ptr PluginManager::addFourOSCToTrack(te::AudioTrack& track)
{
    if (auto p = createFourOSC())
    {
        track.pluginList.insertPlugin (*p, 0, nullptr);
        return p;
    }
    return {};
}