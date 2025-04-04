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

    auto track = te::getAudioTracks(edit)[trackIndex];

    te::TimeRange oneBar(0s, edit.tempoSequence.toTime({1, te::BeatDuration()}));

    auto clip = track->insertNewClip(te::TrackItem::Type::midi, "Midi Clip", oneBar, nullptr);
    auto midiClip = dynamic_cast<te::MidiClip*>(clip);

    if (midiClip == nullptr)
        return;
    int offset = trackIndex + 20;

    midiClip->getSequence().addNote(60 + offset, 0_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(64 + offset, 1_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(67 + offset, 2_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(72 + offset, 3_bp, 0.5_bd, 100, 0, nullptr);


    if (track->pluginList.size() == 0)
    {
        auto plugin = edit.getPluginCache().createNewPlugin(te::FourOscPlugin::xmlTypeName, {}).get();
        if (plugin)
            track->pluginList.insertPlugin(*plugin, 0, nullptr);
        std::unique_ptr<juce::Component> pluginUI = plugin->createEditor();
    }
}
