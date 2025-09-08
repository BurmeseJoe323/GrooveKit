#pragma once

#include <tracktion_engine/tracktion_engine.h>

#include "TrackManager.h"
#include "../MIDIEngine/MIDIEngine.h"
#include "../AudioEngine/AudioEngine.h"

namespace IDs
{
#define DECLARE_ID(name)  const juce::Identifier name (#name);
    DECLARE_ID (EDITVIEWSTATE)
    DECLARE_ID (showMasterTrack)
    DECLARE_ID (showGlobalTrack)
    DECLARE_ID (showMarkerTrack)
    DECLARE_ID (showChordTrack)
    DECLARE_ID (showMidiDevices)
    DECLARE_ID (showWaveDevices)
    DECLARE_ID (viewX1)
    DECLARE_ID (viewX2)
    DECLARE_ID (viewY)
    DECLARE_ID (drawWaveforms)
    DECLARE_ID (showHeaders)
    DECLARE_ID (showFooters)
    DECLARE_ID (showArranger)
    #undef DECLARE_ID
}

namespace te = tracktion;

using namespace te::literals;
using namespace std::literals;

class EditViewState
{
public:
    EditViewState (te::Edit& e, te::SelectionManager& s)
        : edit (e), selectionManager (s)
    {
        state = edit.state.getOrCreateChildWithName (IDs::EDITVIEWSTATE, nullptr);

        auto um = &edit.getUndoManager();

        showMasterTrack.referTo (state, IDs::showMasterTrack, um, false);
        showGlobalTrack.referTo (state, IDs::showGlobalTrack, um, false);
        showMarkerTrack.referTo (state, IDs::showMarkerTrack, um, false);
        showChordTrack.referTo (state, IDs::showChordTrack, um, false);
        showArrangerTrack.referTo (state, IDs::showArranger, um, false);
        drawWaveforms.referTo (state, IDs::drawWaveforms, um, true);
        showHeaders.referTo (state, IDs::showHeaders, um, true);
        showFooters.referTo (state, IDs::showFooters, um, false);
        showMidiDevices.referTo (state, IDs::showMidiDevices, um, false);
        showWaveDevices.referTo (state, IDs::showWaveDevices, um, true);

        viewX1.referTo (state, IDs::viewX1, um, 0s);
        viewX2.referTo (state, IDs::viewX2, um, 15s);
        viewY.referTo (state, IDs::viewY, um, 0);
    }

    int timeToX (te::TimePosition time, int width) const
    {
        return juce::roundToInt (((time - viewX1) * width) / (viewX2 - viewX1));
    }

    te::TimePosition xToTime (int x, int width) const
    {
        return te::toPosition ((viewX2 - viewX1) * (double (x) / width)) + te::toDuration (viewX1.get());
    }

    te::TimePosition beatToTime (te::BeatPosition b) const
    {
        auto& ts = edit.tempoSequence;
        return ts.toTime (b);
    }

    te::Edit& edit;
    te::SelectionManager& selectionManager;

    juce::CachedValue<bool> showMasterTrack, showGlobalTrack, showMarkerTrack, showChordTrack, showArrangerTrack,
                      drawWaveforms, showHeaders, showFooters, showMidiDevices, showWaveDevices;

    juce::CachedValue<te::TimePosition> viewX1, viewX2;
    juce::CachedValue<double> viewY;

    juce::ValueTree state;
};


class AppEngine
{
public:
    AppEngine();
    ~AppEngine();

    void createOrLoadEdit();
    void play();
    void stop();

    int addMidiTrack();
    int getNumTracks();
    void deleteMidiTrack(int index);
    void addMidiClipToTrack(int trackIndex);

    te::MidiList &getMidiClipFromTrack(int trackIndex);

    void setTrackMuted(int index, bool mute) { trackManager->setTrackMuted(index, mute); }
    bool isTrackMuted(int index) const { return trackManager->isTrackMuted(index); }

    void soloTrack(int index);
    void setTrackSoloed(int index, bool solo);
    bool isTrackSoloed(int index) const;
    bool anyTrackSoloed() const;

    EditViewState& getEditViewState();
    te::Edit &getEdit();

private:
    std::unique_ptr<tracktion::engine::Engine> engine;
    std::unique_ptr<tracktion::engine::Edit> edit;
    std::unique_ptr<te::SelectionManager> selectionManager;

    std::unique_ptr<EditViewState> editViewState;

    std::unique_ptr<MIDIEngine> midiEngine;
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<TrackManager> trackManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppEngine)
};