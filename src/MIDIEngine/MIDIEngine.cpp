#include "MIDIEngine.h"

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

MIDIEngine::MIDIEngine(te::Edit& editRef)
    : edit(editRef)
{
}

void MIDIEngine::addMidiTrack()

{
    int currentNumTracks = getAudioTracks(edit).size();
    edit.ensureNumberOfAudioTracks(currentNumTracks + 1);
}

void MIDIEngine::addMidiClipToTrack(int trackIndex)
{
    auto audioTracks = getAudioTracks(edit);
    if (trackIndex < 0 || trackIndex >= audioTracks.size())
        return;

    auto track = audioTracks[trackIndex];

    te::TimeRange oneBar(0s, edit.tempoSequence.toTime({1, te::BeatDuration()}));

    auto clip = track->insertNewClip(te::TrackItem::Type::midi, "Midi Clip", oneBar, nullptr);
    auto midiClip = dynamic_cast<te::MidiClip*>(clip);

    if (midiClip == nullptr)
        return;

    midiClip->getSequence().addNote(60, 0_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(64, 1_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(67, 2_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(72, 3_bp, 0.5_bd, 100, 0, nullptr);

    auto plugin = edit.getPluginCache().createNewPlugin(te::FourOscPlugin::xmlTypeName, {}).get();
    if (plugin)
        track->pluginList.insertPlugin(*plugin, 0, nullptr);
}
