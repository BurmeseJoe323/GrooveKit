#include "MIDIEngine.h"

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

MIDIEngine::MIDIEngine(te::Edit& editRef)
    : edit(editRef)
{
}

int MIDIEngine::addMidiTrack()

{
    int currentNumTracks = getAudioTracks(edit).size();
    edit.ensureNumberOfAudioTracks(currentNumTracks + 1);
    return currentNumTracks + 1;
}

void MIDIEngine::addMidiClipToTrack(int trackIndex)
{
    auto audioTracks = getAudioTracks(edit);
    if (trackIndex < 0 || trackIndex >= audioTracks.size())
        return;

    auto track = te::getAudioTracks(edit)[trackIndex];

    // Define a time range of one bar
    te::TimeRange oneBar(0s, edit.tempoSequence.toTime({1, te::BeatDuration()}));

    // Insert a new MIDI clip
    auto clip = track->insertNewClip(te::TrackItem::Type::midi, "Midi Clip", oneBar, nullptr);
    auto midiClip = dynamic_cast<te::MidiClip*>(clip);
    DBG("Clip added to track: " << trackIndex);

    if (midiClip == nullptr)
        return;

    switch (trackIndex)
    {
        case 1:  // Bass
            midiClip->getSequence().addNote(36, 0_bp, 1_bd, 100, 0, nullptr);
            midiClip->getSequence().addNote(40, 1_bp, 1_bd, 100, 0, nullptr);
            midiClip->getSequence().addNote(43, 2_bp, 1_bd, 100, 0, nullptr);
            break;
        case 2:  // Chords
            midiClip->getSequence().addNote(60, 0_bp, 1_bd, 100, 0, nullptr);
            midiClip->getSequence().addNote(64, 0_bp, 1_bd, 100, 0, nullptr);
            midiClip->getSequence().addNote(67, 0_bp, 1_bd, 100, 0, nullptr);
            break;
        case 3:  // melody
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
