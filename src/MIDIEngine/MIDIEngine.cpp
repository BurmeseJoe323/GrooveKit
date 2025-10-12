// JUNIE
#include "MIDIEngine.h"

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

MIDIEngine::MIDIEngine (te::Edit& editRef)
    : edit (editRef)
{
}

void MIDIEngine::addMidiClipToTrack (const int trackIndex) const
{
    auto audioTracks = getAudioTracks (edit);
    if (trackIndex < 0 || trackIndex >= (int) audioTracks.size())
        return;

    auto* track = te::getAudioTracks (edit)[static_cast<size_t> (trackIndex)];

    // Determine start position: place new clip after the last existing MIDI clip on this track
    double startBeats = 0.0;
    for (auto* c : track->getClips())
    {
        if (c != nullptr && c->isMidi())
        {
            if (const auto* mc = dynamic_cast<te::MidiClip*> (c))
            {
                const double s = mc->getStartBeat().inBeats();
                const double len = mc->getLengthInBeats().inBeats();
                startBeats = juce::jmax (startBeats, s + len);
            }
        }
    }

    // Use a smaller default length for new empty clips (1 bar = 4 beats)
    constexpr double lengthBeats = 4.0;

    const auto startPos = edit.tempoSequence.toTime (te::BeatPosition::fromBeats (startBeats));
    const auto endPos = edit.tempoSequence.toTime (te::BeatPosition::fromBeats (startBeats + lengthBeats));
    te::TimeRange range { startPos, endPos };

    auto* clip = track->insertNewClip (te::TrackItem::Type::midi, "Midi Clip", range, nullptr);
    auto* midiClip = dynamic_cast<te::MidiClip*> (clip);
    juce::ignoreUnused (midiClip);
    DBG ("Clip added to track: " << trackIndex << ", startBeat=" << startBeats << ", lengthBeats=" << lengthBeats);

    // midiClip->setStart(true, true);
    // midiClip->setLength(true, true);

    if (midiClip == nullptr)
        return;

    // switch (trackIndex)
    // {
    //     case 0:
    //         midiClip->getSequence().addNote(36, 0_bp, 0.5_bd, 112, 0, nullptr);
    //         midiClip->getSequence().addNote(36, 4_bp, 0.5_bd, 112, 0, nullptr);
    //
    //         // Snare (per your mapping) on 2 & 4 (beats 2 and 6)
    //         midiClip->getSequence().addNote(37, 2_bp, 0.5_bd, 108, 0, nullptr);
    //         midiClip->getSequence().addNote(37, 6_bp, 0.5_bd, 108, 0, nullptr);
    //
    //         // Hi-hat 8ths across 2 bars (every 1/2 beat)
    //         for (int i = 0; i < 16; ++i)
    //         {
    //             auto start = te::BeatPosition::fromBeats(0.5 * i);   // 0.0, 0.5, 1.0, ..., 7.5
    //             midiClip->getSequence().addNote(38, start, te::BeatDuration::fromBeats(0.25), 92, 0, nullptr);
    //         }
    //         break;
    //     case 1:  // Bass
    //         midiClip->getSequence().addNote(36, 0_bp, 1_bd, 100, 0, nullptr);
    //         midiClip->getSequence().addNote(40, 1_bp, 1_bd, 100, 0, nullptr);
    //         midiClip->getSequence().addNote(43, 2_bp, 1_bd, 100, 0, nullptr);
    //         break;
    //     case 2:  // Chords
    //         midiClip->getSequence().addNote(60, 0_bp, 1_bd, 100, 0, nullptr);
    //         midiClip->getSequence().addNote(64, 0_bp, 1_bd, 100, 0, nullptr);
    //         midiClip->getSequence().addNote(67, 0_bp, 1_bd, 100, 0, nullptr);
    //         break;
    //     case 3:  // melody
    //         midiClip->getSequence().addNote(76, 0_bp, 0.5_bd, 100, 0, nullptr);
    //         midiClip->getSequence().addNote(79, 1_bp, 0.5_bd, 100, 0, nullptr);
    //         midiClip->getSequence().addNote(81, 2_bp, 0.5_bd, 100, 0, nullptr);
    //         break;
    //     default:
    //         midiClip->getSequence().addNote(72, 0_bp, 1_bd, 100, 0, nullptr);
    //         midiClip->getSequence().addNote(74, 1_bp, 1_bd, 100, 0, nullptr);
    //         break;
    // }

    edit.getTransport().ensureContextAllocated();
    edit.restartPlayback();
}

te::MidiClip* MIDIEngine::getMidiClipFromTrack (const int trackIndex) const
{
    auto audioTracks = getAudioTracks (edit);
    if (trackIndex < 0 || trackIndex >= (int) audioTracks.size())
        return nullptr;

    const auto* track = te::getAudioTracks (edit)[static_cast<size_t> (trackIndex)];
    auto* clip = track->getClips().getFirst();

    if (clip == nullptr || !clip->isMidi())
        return nullptr;

    return dynamic_cast<te::MidiClip*> (clip);
}

std::vector<te::MidiClip*> MIDIEngine::getMidiClipsFromTrack (const int trackIndex) const
{
    std::vector<te::MidiClip*> midiClips;

    auto audioTracks = getAudioTracks (edit);
    if (trackIndex < 0 || trackIndex >= (int) audioTracks.size())
        return midiClips;

    const auto* track = te::getAudioTracks (edit)[static_cast<size_t> (trackIndex)];
    auto& clips = track->getClips();

    for (auto* c : clips)
        if (c != nullptr && c->isMidi())
            if (auto* mc = dynamic_cast<te::MidiClip*> (c))
                midiClips.push_back (mc);

    return midiClips;
}
