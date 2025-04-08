#include "MIDIEngine.h"

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

MIDIEngine::MIDIEngine(te::Edit& editRef)
    : edit(editRef)
{
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
    DBG("Clip added to track: " << trackIndex);

    if (midiClip == nullptr)
        return;
    int offset = 0 + trackIndex;

    midiClip->getSequence().addNote(60 + offset, 0_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(64 + offset, 1_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(67 + offset, 2_bp, 0.5_bd, 100, 0, nullptr);
    midiClip->getSequence().addNote(72 + offset, 3_bp, 0.5_bd, 100, 0, nullptr);

    edit.restartPlayback();


}
