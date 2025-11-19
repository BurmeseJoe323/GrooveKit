#include "MIDIEngine.h"

namespace te = tracktion::engine;
namespace t = tracktion;
using namespace std::literals;
using namespace t::literals;

MIDIEngine::MIDIEngine (te::Edit& editRef)
    : edit (editRef)
{
}

bool MIDIEngine::addMidiClipToTrackAt(int trackIndex,
                                      t::TimePosition start,
                                      t::BeatDuration length)
{
    auto tracks = te::getAudioTracks (edit);
    if (!juce::isPositiveAndBelow (trackIndex, tracks.size()))
        return false;

    auto* track = tracks.getUnchecked (trackIndex);

    const auto startTime = start;
    const auto startBeat = edit.tempoSequence.toBeats (startTime);
    const auto endBeat = startBeat + length;
    const auto endTime = edit.tempoSequence.toTime (endBeat);

    t::TimeRange range { startTime, endTime };

    if (track->insertNewClip (te::TrackItem::Type::midi, "MIDI", range, nullptr))
    {
        DBG ("Added MIDI clip @" << startTime.inSeconds()
                                 << "s len(beats)=" << length.inBeats());
        return true;
    }

    return false;
}

bool MIDIEngine::addMidiClipToTrack (int trackIndex)
{
    auto tracks = getAudioTracks (edit);
    if (!juce::isPositiveAndBelow (trackIndex, tracks.size()))
        return false;

    auto* track = tracks.getUnchecked (trackIndex);
    auto clips = track->getClips();

    // default length = 4 beats
    const auto defLenBeats = 4_bd;

    // place at playhead OR just after the last existing clip end (whichever is later)
    const auto playhead = edit.getTransport().getPosition();
    t::TimePosition nextPos = playhead;

    if (clips.size() > 0)
    {
        double lastEnd = 0.0;
        for (auto* c : clips)
            lastEnd = juce::jmax (lastEnd, c->getPosition().time.getEnd().inSeconds());

        nextPos = t::TimePosition::fromSeconds(juce::jmax(lastEnd, playhead.inSeconds()));
    }
    // don’t force restart every time; leave transport state alone
    edit.getTransport().ensureContextAllocated();

    return addMidiClipToTrackAt (trackIndex, nextPos, defLenBeats);
}

juce::Array<te::MidiClip*> MIDIEngine::getMidiClipsFromTrack (int trackIndex)
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

bool MIDIEngine::importMidiFileToTrack (const juce::File& midiFile,
                                        int trackIndex,
                                        t::TimePosition destStart)
{
    DBG ("[MIDIEngine] importMidiFileToTrack: " << midiFile.getFullPathName()
         << " trackIndex=" << trackIndex
         << " destStart=" << destStart.inSeconds() << "s");

    if (! midiFile.existsAsFile())
    {
        DBG ("[MIDIEngine] File doesn't exist");
        return false;
    }

    auto inputStream = std::unique_ptr<juce::FileInputStream> (midiFile.createInputStream());
    if (inputStream == nullptr || ! inputStream->openedOk())
    {
        DBG ("[MIDIEngine] Failed to open input stream");
        return false;
    }

    juce::MidiFile mf;
    if (! mf.readFrom (*inputStream))
    {
        DBG ("[MIDIEngine] MidiFile::readFrom failed");
        return false;
    }

    const int timeFormat = mf.getTimeFormat();
    if (timeFormat <= 0)
    {
        DBG ("[MIDIEngine] Unsupported time format (SMPTE) or <= 0");
        return false;
    }

    DBG ("[MIDIEngine] Num tracks in file: " << mf.getNumTracks()
         << " PPQ=" << timeFormat);

    // --- Build merged sequence in ticks (note on/off only) ---
    juce::MidiMessageSequence tickSeq;

    for (int tIndex = 0; tIndex < mf.getNumTracks(); ++tIndex)
    {
        if (auto* trackSeq = mf.getTrack (tIndex))
        {
            for (int i = 0; i < trackSeq->getNumEvents(); ++i)
            {
                auto* ev  = trackSeq->getEventPointer (i);
                auto  msg = ev->message;

                if (msg.isNoteOnOrOff())
                    tickSeq.addEvent (msg);
            }
        }
    }

    tickSeq.updateMatchedPairs();

    if (tickSeq.getNumEvents() == 0)
    {
        DBG ("[MIDIEngine] Sequence has no note events");
        return false;
    }

    // --- Normalise ticks so first event is at 0 ticks ---
    const double firstTick = tickSeq.getEventTime (0);
    for (int i = 0; i < tickSeq.getNumEvents(); ++i)
    {
        auto* ev  = tickSeq.getEventPointer (i);
        auto  msg = ev->message;
        msg.setTimeStamp (msg.getTimeStamp() - firstTick);
        ev->message = msg;
    }

    const double endTick = tickSeq.getEndTime();
    if (endTick <= 0.0)
    {
        DBG ("[MIDIEngine] endTick <= 0");
        return false;
    }

    // --- Convert ticks -> seconds using the edit tempo ---
    auto& tempoSeq = edit.tempoSequence;

    // GrooveKit uses a single global tempo, so just grab the first one.
    double bpm = 120.0;
    if (auto* tempo = tempoSeq.getTempo (0))
        bpm = tempo->getBpm();

    const double secondsPerBeat = 60.0 / bpm;

    // ticks -> beats -> seconds
    const double lengthBeats    = endTick / (double) timeFormat;
    const double lengthSeconds  = lengthBeats * secondsPerBeat;

    DBG ("[MIDIEngine] lengthBeats="   << lengthBeats
         << " lengthSeconds="          << lengthSeconds
         << " (endTick="               << endTick
         << ", bpm="                   << bpm << ")");

    // --- Build a sequence whose timestamps are SECONDS from clip start ---
    juce::MidiMessageSequence secondsSeq;

    for (int i = 0; i < tickSeq.getNumEvents(); ++i)
    {
        auto* ev  = tickSeq.getEventPointer (i);
        auto  msg = ev->message;

        const double ticks  = msg.getTimeStamp();
        const double beats  = ticks / (double) timeFormat;      // quarter-note beats
        const double secs   = beats * secondsPerBeat;           // seconds from clip start

        msg.setTimeStamp (secs);
        secondsSeq.addEvent (msg);
    }

    secondsSeq.updateMatchedPairs();

    if (secondsSeq.getNumEvents() == 0)
    {
        DBG ("[MIDIEngine] secondsSeq has no events after tick->sec conversion");
        return false;
    }

    // --- Find target audio track ---
    auto audioTracks = te::getAudioTracks (edit);
    if (! juce::isPositiveAndBelow (trackIndex, audioTracks.size()))
    {
        DBG ("[MIDIEngine] trackIndex out of range for audioTracks");
        return false;
    }

    auto* audioTrack = audioTracks.getUnchecked (trackIndex);
    if (audioTrack == nullptr)
    {
        DBG ("[MIDIEngine] audioTracks[trackIndex] is null");
        return false;
    }

    // Clip lives from destStart .. destStart + lengthSeconds (all in seconds)
    const auto clipStartTime = destStart;
    const auto clipEndTime   = destStart + t::TimeDuration::fromSeconds (lengthSeconds);
    t::TimeRange range { clipStartTime, clipEndTime };

    DBG ("[MIDIEngine] clipStartTime=" << clipStartTime.inSeconds()
         << " clipEndTime="            << clipEndTime.inSeconds()
         << " (lenSeconds="            << lengthSeconds << ")");

    if (auto* baseClip = audioTrack->insertNewClip (te::TrackItem::Type::midi,
                                                    "MIDI",
                                                    range,
                                                    nullptr))
    {
        if (auto* midiClip = dynamic_cast<te::MidiClip*> (baseClip))
        {
            DBG ("[MIDIEngine] insertNewClip -> MidiClip OK, merging "
                 << secondsSeq.getNumEvents() << " events (seconds)");

            midiClip->mergeInMidiSequence (secondsSeq,
                                           te::MidiList::NoteAutomationType::none);

            DBG ("[MIDIEngine] MidiClip import complete. "
                 << "pos start=" << midiClip->getPosition().time.getStart().inSeconds()
                 << " end="      << midiClip->getPosition().time.getEnd().inSeconds());

            return true;
        }

        DBG ("[MIDIEngine] insertNewClip didn't return a MidiClip");
    }
    else
    {
        DBG ("[MIDIEngine] insertNewClip returned null");
    }

    return false;
}