#include "TrackManager.h"
#include <tracktion_engine/tracktion_engine.h>
TrackManager::TrackManager(te::Edit& editRef)
    : edit(editRef) {
}

TrackManager::~TrackManager() = default;

int TrackManager::getNumTracks() const {
    return getAudioTracks(edit).size();
}

te::AudioTrack* TrackManager::getTrack(int index) {
    auto audioTracks = getAudioTracks(edit);
    auto track = audioTracks[index];
    return track;
}

te::AudioTrack *TrackManager::addTrack() {
    juce::Logger::outputDebugString("Track added");
    int currentNumTracks = te::getAudioTracks(edit).size();
    edit.ensureNumberOfAudioTracks(currentNumTracks + 1);

    auto track = te::getAudioTracks(edit)[currentNumTracks];

    auto plugin = edit.getPluginCache().createNewPlugin(te::FourOscPlugin::xmlTypeName, {}).get();
    if (plugin){
        track->pluginList.insertPlugin(*plugin, 0, nullptr);
        te::PluginInitialisationInfo info;
        info.sampleRate = edit.engine.getDeviceManager().getSampleRate();
        info.blockSizeSamples = edit.engine.getDeviceManager().getBlockSize();

        plugin->initialise(info);

        DBG("Plugin on track: " << plugin->getName());
    }
    edit.restartPlayback();
}

void TrackManager::deleteTrack(int index) {
    if (index < 0 || index >= getNumTracks())
        //should probably throw error or something
        return;
    te::AudioTrack* track = getTrack(index);
    edit.deleteTrack(track);
}




