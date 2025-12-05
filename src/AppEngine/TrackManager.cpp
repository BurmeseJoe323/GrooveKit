#include "TrackManager.h"
#include <tracktion_engine/tracktion_engine.h>
#include "../DrumSamplerEngine/DrumSamplerEngineAdapter.h"
#include "../UI/Plugins/Synthesizer/MorphSynthPlugin.h"
#include "../PluginManager/PluginManager.h"

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

    // Resize vectors instead of clearing to preserve existing adapters
    types.resize(n, TrackType::Instrument);
    drumEngines.resize(n);

    for (int i = 0; i < n; ++i)
    {
        auto* track = audioTracks[(size_t) i];

        const bool isDrum = (bool) track->state.getProperty(GKIDs::isDrum, false);
        if (isDrum)
        {
            types[(size_t) i] = TrackType::Drum;
            // Only create adapter if it doesn't already exist
            if (!drumEngines[(size_t) i])
            {
                drumEngines[(size_t) i] = std::make_unique<DrumSamplerEngineAdapter>(edit.engine, *track);
            }
        }
        else
        {
            types[(size_t) i] = TrackType::Instrument;
            // Clear drum adapter if track is not a drum track
            drumEngines[(size_t) i].reset();
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

    // Name based on overall track position (1-indexed)
    track->setName ("Drums " + juce::String (newIndex + 1));

    auto adapter = std::make_unique<DrumSamplerEngineAdapter>(edit.engine, *track);

    // syncBookkeepingToEngine will create the adapter if needed
    syncBookkeepingToEngine();
    edit.getTransport().ensureContextAllocated();
    return newIndex;
}

void TrackManager::clearInstrumentSlot0 (int trackIndex)
{
    if (trackIndex < 0 || trackIndex >= getNumTracks())
        return;

    if (auto* track = getTrack (trackIndex))
    {
        if (track->pluginList.size() == 0)
            return;

        if (auto* p = track->pluginList[0])
        {
            // Only remove if it’s an instrument we consider “the instrument slot”
            if (dynamic_cast<te::ExternalPlugin*> (p)
             || dynamic_cast<MorphSynthPlugin*> (p)
             || dynamic_cast<te::FourOscPlugin*> (p))
            {
                p->deleteFromParent();  // safe TE removal
            }
        }
    }
}

int TrackManager::addInstrumentTrack()
{
    const int newIndex = getNumTracks();
    edit.ensureNumberOfAudioTracks(newIndex + 1);

    auto* track = te::getAudioTracks(edit)[(size_t) newIndex];
    track->state.setProperty (GKIDs::isDrum, false, nullptr);
    // Set default instrument track name (Written by Claude Code)
    track->setName ("Track " + juce::String (newIndex + 1));

    // syncBookkeepingToEngine will set type and clear drum adapter if needed
    syncBookkeepingToEngine();

    // NO default plugin insertion here anymore.

    edit.getTransport().ensureContextAllocated();
    return newIndex;
}

te::Plugin* TrackManager::insertExternalEffect (int trackIndex,
                                                const juce::PluginDescription& desc,
                                                int insertIndex)
{
    if (trackIndex < 0 || trackIndex >= getNumTracks() || pluginManager == nullptr)
        return nullptr;

    if (auto* t = getTrack (trackIndex))
    {
        if (auto p = pluginManager->addExternalEffectToTrack (*t, desc, insertIndex))
            return p.get();
    }
    return nullptr;
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
        if (track->pluginList.size() == 0)
            return nullptr;

        // We only ever treat slot 0 as "the instrument"
        te::Plugin* plug = track->pluginList[0];
        if (plug == nullptr)
            return nullptr;

        // Built-in instruments
        if (auto* morph = dynamic_cast<MorphSynthPlugin*> (plug))
            return morph;

        if (auto* four = dynamic_cast<te::FourOscPlugin*> (plug))
            return four;

        // External instrument (VST, AU, etc.)
        if (auto* ext = dynamic_cast<te::ExternalPlugin*> (plug))
        {
            if (auto* pi = ext->getAudioPluginInstance())
            {
                const auto& desc = pi->getPluginDescription();
                const bool isInstr = desc.isInstrument || pi->acceptsMidi();

                // Only treat it as an instrument if it *really* behaves like one
                if (isInstr)
                    return plug;

                // Not an instrument → ignore, behave as "no instrument"
                return nullptr;
            }

            // Instance not created yet, but if we inserted an external instrument
            // we always put it at index 0, so treat this as the instrument.
            return plug;
        }

        // Anything else at slot 0 isn't considered an instrument
        return nullptr;
    }

    return nullptr;
}


te::Plugin* TrackManager::insertExternalInstrument (int trackIndex, const juce::PluginDescription& desc)
{
    if (trackIndex < 0 || trackIndex >= getNumTracks() || pluginManager == nullptr)
        return nullptr;

    if (auto* t = getTrack (trackIndex))
    {
        if (auto p = pluginManager->addExternalInstrumentToTrack (*t, desc, 0))
            return p.get();
    }
    return nullptr;
}

te::Plugin* TrackManager::insertMorphSynth (int trackIndex)
{
    if (trackIndex < 0 || trackIndex >= getNumTracks())
        return nullptr;

    if (auto* t = getTrack (trackIndex))
    {
        if (auto plugin = edit.getPluginCache().createNewPlugin (MorphSynthPlugin::pluginType, {}))
        {
            t->pluginList.insertPlugin (std::move (plugin), 0, nullptr);
            for (auto* p : t->pluginList)
                if (auto* morph = dynamic_cast<MorphSynthPlugin*> (p))
                    return morph;
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

void TrackManager::clearFxInsertSlot (int trackIndex, int slotIndex)
{
    if (trackIndex < 0 || trackIndex >= getNumTracks())
        return;

    auto* t = getTrack (trackIndex);
    if (! t)
        return;

    const int base = getFxInsertBaseIndex (trackIndex);
    const int pluginIndex = base + slotIndex;

    if (pluginIndex < 0 || pluginIndex >= t->pluginList.size())
        return;

    if (auto* plug = t->pluginList[pluginIndex])
        plug->deleteFromParent();
}

int TrackManager::getFxInsertBaseIndex (int trackIndex) const
{
    if (trackIndex < 0 || trackIndex >= getNumTracks())
        return 0;

    auto audioTracks = getAudioTracks (edit);
    auto* t = audioTracks[(size_t) trackIndex];
    if (! t)
        return 0;

    if (t->pluginList.size() == 0)
        return 0;

    auto* p0 = t->pluginList[0];

    const bool isInstrument =
        dynamic_cast<MorphSynthPlugin*> (p0) != nullptr
        || dynamic_cast<te::FourOscPlugin*> (p0) != nullptr
        || (dynamic_cast<te::ExternalPlugin*> (p0) != nullptr
            && static_cast<te::ExternalPlugin*> (p0)->isSynth());

    // Instrument at [0] ⇒ [1]=volume, [2]=meter, inserts start at [3]
    // No instrument     ⇒ [0]=volume, [1]=meter, inserts start at [2]
    return isInstrument ? 3 : 2;
}
