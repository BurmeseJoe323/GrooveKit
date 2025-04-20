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
    int patternIndex = trackIndex % 4;
    switch (patternIndex)
    {
        DBG("Pattern index: " << patternIndex);
        case 0:  // Bass
            midiClip->getSequence().addNote(36, 0_bp, 1_bd, 100, 0, nullptr);
        midiClip->getSequence().addNote(40, 1_bp, 1_bd, 100, 0, nullptr);
        midiClip->getSequence().addNote(43, 2_bp, 1_bd, 100, 0, nullptr);
        break;
        case 1:  // Chords
            midiClip->getSequence().addNote(60, 0_bp, 1_bd, 100, 0, nullptr);
        midiClip->getSequence().addNote(64, 0_bp, 1_bd, 100, 0, nullptr);
        midiClip->getSequence().addNote(67, 0_bp, 1_bd, 100, 0, nullptr);
        break;
        case 2:  // melody
            midiClip->getSequence().addNote(76, 0_bp, 0.5_bd, 100, 0, nullptr);
        midiClip->getSequence().addNote(79, 1_bp, 0.5_bd, 100, 0, nullptr);
        midiClip->getSequence().addNote(81, 2_bp, 0.5_bd, 100, 0, nullptr);
        break;
        default:
            midiClip->getSequence().addNote(72, 0_bp, 1_bd, 100, 0, nullptr);
        midiClip->getSequence().addNote(74, 1_bp, 1_bd, 100, 0, nullptr);
        break;
    }

    edit.getTransport().ensureContextAllocated();
    edit.restartPlayback();


}
