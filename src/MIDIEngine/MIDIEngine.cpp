#include "MIDIEngine.h"

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

MIDIEngine::MIDIEngine(te::Edit& editRef)
    : edit(editRef)
{
}

void MIDIEngine::addMidiClipToTrack (int trackIndex)
{
    auto tracks = te::getAudioTracks (edit);
    if (trackIndex < 0 || trackIndex >= (int) tracks.size())
        return;

    auto* track = tracks[(size_t) trackIndex];

    auto endPos  = edit.tempoSequence.toTime (8_bp);
    te::TimeRange twoBars { 0s, endPos };

    if (auto c = track->insertNewClip (te::TrackItem::Type::midi, "MIDI Clip", twoBars, nullptr))
    {
        if (auto* midiClip = dynamic_cast<te::MidiClip*> (c))
        {
            // Default to channel 1 so external synths respond.
            // (If you want drums on 10, detect your drum track and set chan = 10.)
            const int chan = 1;

            switch (trackIndex)
            {
                case 0: // drums-style pattern (still OK on ch1)
                    midiClip->getSequence().addNote (36, 0_bp, 0.5_bd, 112, chan, nullptr);
                    midiClip->getSequence().addNote (36, 4_bp, 0.5_bd, 112, chan, nullptr);
                    midiClip->getSequence().addNote (37, 2_bp, 0.5_bd, 108, chan, nullptr);
                    midiClip->getSequence().addNote (37, 6_bp, 0.5_bd, 108, chan, nullptr);
                    for (int i = 0; i < 16; ++i)
                        midiClip->getSequence().addNote (38,
                            te::BeatPosition::fromBeats (0.5 * i),
                            te::BeatDuration::fromBeats (0.25),
                            92, chan, nullptr);
                    break;

                case 1: // bass
                    midiClip->getSequence().addNote (36, 0_bp, 1_bd, 100, chan, nullptr);
                    midiClip->getSequence().addNote (40, 1_bp, 1_bd, 100, chan, nullptr);
                    midiClip->getSequence().addNote (43, 2_bp, 1_bd, 100, chan, nullptr);
                    break;

                case 2: // chords
                    midiClip->getSequence().addNote (60, 0_bp, 1_bd, 100, chan, nullptr);
                    midiClip->getSequence().addNote (64, 0_bp, 1_bd, 100, chan, nullptr);
                    midiClip->getSequence().addNote (67, 0_bp, 1_bd, 100, chan, nullptr);
                    break;

                case 3: // melody
                    midiClip->getSequence().addNote (76, 0_bp, 0.5_bd, 100, chan, nullptr);
                    midiClip->getSequence().addNote (79, 1_bp, 0.5_bd, 100, chan, nullptr);
                    midiClip->getSequence().addNote (81, 2_bp, 0.5_bd, 100, chan, nullptr);
                    break;

                default:
                    midiClip->getSequence().addNote (72, 0_bp, 1_bd, 100, chan, nullptr);
                    midiClip->getSequence().addNote (74, 1_bp, 1_bd, 100, chan, nullptr);
                    break;
            }

            // if (auto* mc = dynamic_cast<te::MidiClip*> (c.get()))
            // {
            //     auto& seq = mc->getSequence();
            //     if (seq.getNumNotes() > 0)
            //     {
            //         auto* n = seq.getNote (0);
            //         DBG("[AUDIT] notes=" << seq.getNumNotes()
            //             << " firstChan=" << n->midiChannel
            //             << " pitch=" << n->getNoteNumber());
            //     }
            //     else
            //     {
            //         DBG("[AUDIT] notes=0");
            //     }
            // }


            // Make sure playback graph exists so the instrument instance is built.
            edit.getTransport().ensureContextAllocated();
            // If you want to auto-audition:
            // edit.getTransport().setPosition (0_tp);
            // edit.getTransport().play (false);
        }
    }

    DBG ("Clip added to track: " << trackIndex);
}

te::MidiClip *MIDIEngine::getMidiClipFromTrack(int trackIndex) {
    auto audioTracks = getAudioTracks(edit);
    if (trackIndex < 0 || trackIndex >= audioTracks.size())
        // Return nullptr if the trackIndex is invalid
        return nullptr;

    auto track = te::getAudioTracks(edit)[trackIndex];
    auto clip = track->getClips().getFirst();

    if (clip == nullptr) {
        return nullptr;
    } else if (!clip->isMidi()) {
        return nullptr;
    }

    auto *midiClip = dynamic_cast<te::MidiClip*>(clip);
    return midiClip;
}


