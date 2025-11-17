#include "TrackManager.h"
#include <tracktion_engine/tracktion_engine.h>
#include "../DrumSamplerEngine/DrumSamplerEngineAdapter.h"
#include "../UI/Plugins/Synthesizer/MorphSynthPlugin.h"

namespace {
    static int asIndexChecked (int idx, int size) { return (idx >= 0 && idx < size) ? idx : -1; }
}

namespace GKIDs {
    static const juce::Identifier isDrum ("gk_isDrum");
}

TrackManager::TrackManager(te::Edit& editRef)
    : edit(editRef) {
    syncBookkeepingToEngine();
}

TrackManager::~TrackManager() = default;

void TrackManager::syncBookkeepingToEngine()
{
    auto audioTracks = te::getAudioTracks(edit);
    const int n = (int) audioTracks.size();

    types.clear();        types.resize(n, TrackType::Instrument);
    drumEngines.clear();  drumEngines.resize(n);

    for (int i = 0; i < n; ++i)
    {
        auto* track = audioTracks[(size_t) i];

        const bool isDrum = (bool) track->state.getProperty(GKIDs::isDrum, false);
        if (isDrum)
        {
            types[(size_t) i] = TrackType::Drum;
            drumEngines[(size_t) i] = std::make_unique<DrumSamplerEngineAdapter>(edit.engine, *track);
        }
    }
}

int TrackManager::getNumTracks() const {
    return getAudioTracks(edit).size();
}

te::AudioTrack* TrackManager::getTrack(int index) {
    auto audioTracks = getAudioTracks(edit);
    auto track = audioTracks[index];
    return track;
}

int TrackManager::addDrumTrack()
{
    const int newIndex = getNumTracks();
    edit.ensureNumberOfAudioTracks(newIndex + 1);

    auto* track = te::getAudioTracks(edit)[(size_t) newIndex];
    track->state.setProperty (GKIDs::isDrum, true, nullptr);           // <-- persist

    // Set default drum track name (Written by Claude Code)
    track->setName ("Drums");

    auto adapter = std::make_unique<DrumSamplerEngineAdapter>(edit.engine, *track);

    syncBookkeepingToEngine();
    types[(size_t) newIndex] = TrackType::Drum;
    drumEngines[(size_t) newIndex] = std::move(adapter);
    edit.getTransport().ensureContextAllocated();
    return newIndex;
}

int TrackManager::addInstrumentTrack()
{
    const int newIndex = getNumTracks();
    edit.ensureNumberOfAudioTracks(newIndex + 1);

    auto* track = te::getAudioTracks(edit)[(size_t) newIndex];
    track->state.setProperty (GKIDs::isDrum, false, nullptr);

    // Set default instrument track name (Written by Claude Code)
    track->setName ("Track " + juce::String (newIndex + 1));

    syncBookkeepingToEngine();
    types[(size_t) newIndex] = TrackType::Instrument;
    if ((int) drumEngines.size() > newIndex)
        drumEngines[(size_t) newIndex].reset();

    // if (auto plugin = edit.getPluginCache().createNewPlugin(te::FourOscPlugin::xmlTypeName, {}))
    //     track->pluginList.insertPlugin(std::move(plugin), 0, nullptr);
    if (auto plugin = edit.getPluginCache().createNewPlugin (MorphSynthPlugin::pluginType, {}))
    {
        // Insert MorphSynth into the track
        track->pluginList.insertPlugin (std::move (plugin), 0, nullptr);

        // Safely loop through all plugins on this track and find the MorphSynth instance
        for (auto* p : track->pluginList)
        {
            if (auto* morph = dynamic_cast<MorphSynthPlugin*> (p))
            {
                if (morph->state.isValid())
                    morph->restoreFromValueTree (morph->state);
            }
        }
    }

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
        //should probably throw error or something
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

te::Plugin* TrackManager::getInstrumentPluginOnTrack (int trackIndex)
{
    if (trackIndex < 0 || trackIndex >= getNumTracks())
        return nullptr;

    if (auto* track = getTrack (trackIndex))
    {
        // getPlugins() → juce::Array<Plugin*>
        for (auto* plug : track->pluginList.getPlugins())
        {
            if (!plug) continue;
            DBG("Track " << trackIndex << " plugin: " << plug->getName());

            // We’re targeting the built-in instrument right now
            if (auto* four = dynamic_cast<te::FourOscPlugin*> (plug))
                return four;

            // If later you want “any instrument”, keep a generic path here:
            // if (plug->getType().isInstrument()) return plug;  // only if your build exposes it
        }
    }
    return nullptr;
}

double TrackManager::getClipStartSeconds (int trackIndex, int clipIndex) const
{
    auto audioTracks = te::getAudioTracks(edit);
    if (trackIndex < 0 || trackIndex >= (int)audioTracks.size())
        return 0.0;

    auto* track = audioTracks[(size_t)trackIndex];
    if (!track)
        return 0.0;

    const auto& clips = track->getClips();
    if (clipIndex < 0 || clipIndex >= clips.size())
        return 0.0;

    auto* clip = clips[(size_t)clipIndex];
    if (!clip)
        return 0.0;

    return clip->getPosition().getStart().inSeconds();
}

double TrackManager::getClipLengthSeconds (int trackIndex, int clipIndex) const
{
    auto audioTracks = te::getAudioTracks(edit);
    if (trackIndex < 0 || trackIndex >= (int)audioTracks.size())
        return 0.0;

    auto* track = audioTracks[(size_t)trackIndex];
    if (!track)
        return 0.0;

    const auto& clips = track->getClips();
    if (clipIndex < 0 || clipIndex >= clips.size())
        return 0.0;

    auto* clip = clips[(size_t)clipIndex];
    if (!clip)
        return 0.0;

    return clip->getPosition().getLength().inSeconds();

}













