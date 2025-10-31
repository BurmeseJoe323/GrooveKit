// JUNIE
#include "MIDIEngine.h"

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

MIDIEngine::MIDIEngine (te::Edit& editRef)
    : edit (editRef)
{
}

void MIDIEngine::addMidiClipToTrackAt(int trackIndex,
                                      te::TimePosition start,
                                      te::BeatDuration length)
{
    auto tracks = te::getAudioTracks(edit);
    if (!juce::isPositiveAndBelow(trackIndex, tracks.size())) return;

    auto* track = tracks.getUnchecked(trackIndex);

    const auto startTime = start;

    const auto startBeat = edit.tempoSequence.toBeats(startTime);
    const auto endBeat   = startBeat + length;
    const auto endTime   = edit.tempoSequence.toTime(endBeat);

    te::TimeRange range { startTime, endTime };

    if (auto* clip = track->insertNewClip(te::TrackItem::Type::midi, "MIDI", range, nullptr))
        if (auto* mc = dynamic_cast<te::MidiClip*>(clip))
            DBG("Added MIDI clip @" << startTime.inSeconds()
                                    << "s len(beats)=" << length.inBeats());
}


void MIDIEngine::addMidiClipToTrack(int trackIndex)
{
    auto tracks = getAudioTracks(edit);
    if (!juce::isPositiveAndBelow(trackIndex, tracks.size())) return;

    auto* track = tracks.getUnchecked(trackIndex);
    auto clips  = track->getClips();

    // default length = 8 beats
    const auto defLenBeats = 8_bd;

    // place at playhead OR just after the last existing clip end (whichever is later)
    const auto playhead = edit.getTransport().getPosition();
    te::TimePosition nextPos = playhead;

    if (clips.size() > 0)
    {
        double lastEnd = 0.0;
        for (auto* c : clips)
            lastEnd = juce::jmax(lastEnd, c->getPosition().time.getEnd().inSeconds());

        nextPos = te::TimePosition::fromSeconds(juce::jmax(lastEnd, playhead.inSeconds()));
    }

    addMidiClipToTrackAt(trackIndex, nextPos, defLenBeats);

    edit.getTransport().ensureContextAllocated();
    // don’t force restart every time; leave transport state alone
}

juce::Array<te::MidiClip*> MIDIEngine::getMidiClipsFromTrack(int trackIndex)
{
    juce::Array<te::MidiClip*> midiClips;

    auto audioTracks = getAudioTracks (edit);
    if (trackIndex < 0 || trackIndex >= (int) audioTracks.size())
        return midiClips;

    const auto* track = te::getAudioTracks (edit)[static_cast<size_t> (trackIndex)];
    auto& clips = track->getClips();

    for (auto* c : clips)
        if (c != nullptr && c->isMidi())
            if (auto* mc = dynamic_cast<te::MidiClip*> (c))
                midiClips.add (mc);

    return midiClips;
}

te::MidiClip* MIDIEngine::getMidiClipFromTrack (int trackIndex)
{
    auto audioTracks = getAudioTracks (edit);
    if (trackIndex < 0 || trackIndex >= (int) audioTracks.size())
        return nullptr;

    const auto* track = te::getAudioTracks (edit)[trackIndex];
    auto* clip = track->getClips().getFirst();

    if (clip == nullptr || !clip->isMidi())
        return nullptr;

    return dynamic_cast<te::MidiClip*> (clip);
}
