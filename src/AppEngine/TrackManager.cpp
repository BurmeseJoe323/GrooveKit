#include "TrackManager.h"
#include <tracktion_engine/tracktion_engine.h>
#include "../DrumSamplerEngine/DrumSamplerEngineAdapter.h"

namespace {
    static int asIndexChecked (int idx, int size) { return (idx >= 0 && idx < size) ? idx : -1; }
}

TrackManager::TrackManager(te::Edit& editRef)
    : edit(editRef) {
    syncBookkeepingToEngine();
}

TrackManager::~TrackManager() = default;

void TrackManager::syncBookkeepingToEngine()
{
    const int n = (int) te::getAudioTracks(edit).size();
    types.resize(n, TrackType::Instrument);
    drumEngines.resize(n); // nullptrs for non-drum
}

int TrackManager::getNumTracks() const {
    return getAudioTracks(edit).size();
}

te::AudioTrack* TrackManager::getTrack(int index){
    auto ts = te::getAudioTracks(edit);
    if (auto i = asIndexChecked(index, (int) ts.size()); i >= 0)
        return ts[(size_t) i];
    return nullptr;
}

int TrackManager::addInstrumentTrack()
{
    const int newIndex = getNumTracks();
    edit.ensureNumberOfAudioTracks(newIndex + 1);

    auto* track = te::getAudioTracks(edit)[(size_t) newIndex];

    // Insert FourOsc (same style you already use)
    if (auto plugin = edit.getPluginCache().createNewPlugin(te::FourOscPlugin::xmlTypeName, {}))
        track->pluginList.insertPlugin(std::move(plugin), 0, nullptr);

    // Bookkeeping
    syncBookkeepingToEngine();
    types[(size_t) newIndex] = TrackType::Instrument;

    // Make sure engine is warmed up
    edit.getTransport().ensureContextAllocated();
    return newIndex;
}

int TrackManager::addDrumTrack()
{
    const int newIndex = getNumTracks();
    edit.ensureNumberOfAudioTracks(newIndex + 1);

    auto* track = te::getAudioTracks(edit)[(size_t) newIndex];

    // Build adapter; it will find/create a Sampler plugin under the hood.
    // Access the Engine from the Edit (common Tracktion pattern).
    drumEngines.emplace_back(); // pre-grow to avoid reallocation before sync
    auto adapter = std::make_unique<DrumSamplerEngineAdapter>(edit.engine, *track);

    // Bookkeeping
    syncBookkeepingToEngine(); // ensure vectors sized
    types[(size_t) newIndex] = TrackType::Drum;
    drumEngines[(size_t) newIndex] = std::move(adapter);

    edit.getTransport().ensureContextAllocated();
    return newIndex;
}

int TrackManager::addTrack()
{
    return addInstrumentTrack();
}



void TrackManager::deleteTrack(int index)
{
    if (index < 0 || index >= getNumTracks())
        return;

    if (auto* track = getTrack(index))
    {
        edit.deleteTrack(track);

        if ((int) types.size() > index)      types.erase(types.begin() + index);
        if ((int) drumEngines.size() > index) drumEngines.erase(drumEngines.begin() + index);
    }
}

bool TrackManager::isDrumTrack(int index) const
{
    if (index < 0 || index >= getNumTracks() || (int) types.size() <= index)
        return false;
    return types[(size_t) index] == TrackType::Drum;
}

DrumSamplerEngineAdapter* TrackManager::getDrumAdapter(int index)
{
    if (index < 0 || index >= getNumTracks() || (int) drumEngines.size() <= index)
        return nullptr;
    return drumEngines[(size_t) index].get();
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







