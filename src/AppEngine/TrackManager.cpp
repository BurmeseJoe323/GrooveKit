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

int TrackManager::addTrack()
{
    juce::Logger::outputDebugString("Track added");

    const int currentNumTracks = te::getAudioTracks(edit).size();
    edit.ensureNumberOfAudioTracks(currentNumTracks + 1);

    auto* track = te::getAudioTracks(edit)[currentNumTracks];

    auto plugin = edit.getPluginCache().createNewPlugin(te::FourOscPlugin::xmlTypeName, {});
    if (plugin)
    {
        track->pluginList.insertPlugin(std::move(plugin), 0, nullptr);
        juce::Logger::outputDebugString("addTrack: engine had "
            + juce::String(currentNumTracks) + " audio tracks; new index = "
            + juce::String(currentNumTracks));
        juce::Logger::outputDebugString("Track " + juce::String(currentNumTracks)
            + " plugins: " + juce::String(track->pluginList.size()));

    }

    edit.getTransport().ensureContextAllocated();

    edit.restartPlayback();

    return currentNumTracks;
}



void TrackManager::deleteTrack(int index) {
    if (index < 0 || index >= getNumTracks())
        //should probably throw error or something
        return;
    te::AudioTrack* track = getTrack(index);
    edit.deleteTrack(track);
}

void TrackManager::muteTrack(int index) {
    if (auto* track = getTrack(index)) {
        track->setMute(!track->isMuted(false));
    }
}

bool TrackManager::isTrackMuted(int index) const {
    auto audioTracks = te::getAudioTracks(edit);
    if (index < 0 || index >= (int) audioTracks.size())
        return false;
    return audioTracks[(size_t) index]->isMuted(false);
}

void TrackManager::setTrackMuted(int index, bool mute) {
    if (index < 0 || index >= getNumTracks())
        return;
    if (auto* track = getTrack(index))
        track->setMute(mute);
}

void TrackManager::soloTrack(int index)
{
    if (auto* t = getTrack(index))
        setTrackSoloed(index, ! t->isSolo(false));
}

void TrackManager::setTrackSoloed(int index, bool solo)
{
    if (auto* t = getTrack(index))
        t->setSolo(solo);
}

bool TrackManager::isTrackSoloed(int index) const
{
    auto ts = te::getAudioTracks(edit);
    if (index < 0 || index >= (int) ts.size()) return false;
    return ts[(size_t) index]->isSolo(false);
}

bool TrackManager::anyTrackSoloed() const
{
    auto ts = te::getAudioTracks(edit);
    for (auto* t : ts)
        if (t->isSolo(false)) return true;
    return false;
}







