#pragma once

#include "../AudioEngine/AudioEngine.h"
#include "../MIDIEngine/MIDIEngine.h"
#include "../UI/TrackView/TrackHeaderComponent.h"
#include "TrackManager.h"
#include "MidiListener.h"
#include <tracktion_engine/tracktion_engine.h>


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

namespace te = tracktion::engine;
namespace t = tracktion;

using namespace t::literals;
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

    int timeToX (t::TimePosition time, int width) const
    {
        return juce::roundToInt (((time - viewX1) * width) / (viewX2 - viewX1));
    }

    t::TimePosition xToTime (int x, int width) const
    {
        return t::toPosition ((viewX2 - viewX1) * (double (x) / width)) + t::toDuration (viewX1.get());
    }

    t::TimePosition beatToTime (t::BeatPosition b) const
    {
        auto& ts = edit.tempoSequence;
        return ts.toTime (b);
    }

    te::Edit& edit;
    te::SelectionManager& selectionManager;

    juce::CachedValue<bool> showMasterTrack, showGlobalTrack, showMarkerTrack, showChordTrack, showArrangerTrack,
        drawWaveforms, showHeaders, showFooters, showMidiDevices, showWaveDevices;

    juce::CachedValue<t::TimePosition> viewX1, viewX2;
    juce::CachedValue<double> viewY;

    juce::ValueTree state;
};

class AppEngine : private juce::Timer
{
public:
    AppEngine();
    ~AppEngine() override;

    void createOrLoadEdit();
    void play();
    void stop();
    bool isPlaying() const;

    int addMidiTrack();
    int getNumTracks();
    void deleteMidiTrack (int index);
    bool addMidiClipToTrack (int trackIndex);
    // Add an empty MIDI clip at a specific beat on the given track (Junie)
    bool addMidiClipToTrackAt (int trackIndex, t::TimePosition start, t::BeatDuration length);


    bool isDrumTrack (int index) const;

    DrumSamplerEngineAdapter* getDrumAdapter (int index);

    int addDrumTrack();
    int addInstrumentTrack();

    te::MidiClip* getMidiClipFromTrack (int trackIndex);
    juce::Array<te::MidiClip*> getMidiClipsFromTrack (int trackIndex);

    void setTrackMuted (int index, bool mute) { trackManager->setTrackMuted (index, mute); }
    bool isTrackMuted (int index) const { return trackManager->isTrackMuted (index); }

    void soloTrack (int index);
    void setTrackSoloed (int index, bool solo);
    bool isTrackSoloed (int index) const;
    bool anyTrackSoloed() const;

    double getBpm() const;
    void setBpm (double newBpm);

    TrackManager& getTrackManager()       { return *trackManager; }
    TrackManager* getTrackManagerPtr()    { return trackManager.get(); }

    AudioEngine& getAudioEngine();
    MIDIEngine& getMidiEngine();
    MidiListener& getMidiListener() const { return *midiListener; }
    juce::AudioProcessorValueTreeState& getAPVTS();

    EditViewState& getEditViewState();
    te::Edit& getEdit();

    bool setOutputDevice (const juce::String& name)        { return audioEngine->setOutputDeviceByName (name); }
    bool setDefaultOutputDevice()                          { return audioEngine->setDefaultOutputDevice(); }
    juce::StringArray listOutputDevices()            const { return audioEngine->listOutputDevices(); }
    juce::String getCurrentOutputDeviceName()        const { return audioEngine->getCurrentOutputDeviceName(); }

    // MIDI Input device management
    juce::StringArray listMidiInputDevices()         const { return audioEngine->listMidiInputDevices(); }
    bool connectMidiInputDevice(int deviceIndex)           { return audioEngine->connectMidiInputToCallback(deviceIndex, midiListener.get()); }
    bool setMidiInputDevice(const juce::String& deviceName) { return audioEngine->setMidiInputDeviceByName(deviceName, midiListener.get()); }
    juce::String getCurrentMidiInputDeviceName()     const { return audioEngine->getCurrentMidiInputDeviceName(); }
    void disconnectAllMidiInputs()                         { audioEngine->disconnectAllMidiInputs(midiListener.get()); }

    bool saveEdit();
    void saveEditAsAsync (std::function<void (bool success)> onDone = {});

    bool isDirty() const noexcept;
    const juce::File& getCurrentEditFile() const noexcept { return currentEditFile; }

    void setAutosaveMinutes (int minutes);

    void openEditAsync (std::function<void (bool success)> onDone = {});
    bool loadEditFromFile (const juce::File& file);
    std::function<void()> onEditLoaded;
    std::function<void(double oldBpm, double newBpm, t::TimeRange oldLoopRange, t::TimePosition oldPlayheadPos)> onBpmChanged;

    void newUntitledEdit();

    // Track controller listener registry (Junie)
    void registerTrackListener (int index, TrackHeaderComponent::Listener* l);
    void unregisterTrackListener (int index, TrackHeaderComponent::Listener* l);
    [[nodiscard]] TrackHeaderComponent::Listener* getTrackListener (int index) const;

    void setArmedTrack (int index);
    int getArmedTrackIndex() const;
    te::AudioTrack* getArmedTrack();
    std::function<void()> onArmedTrackChanged;

    void makeFourOscAuditionPatch (int trackIndex);
    void openInstrumentEditor (int trackIndex);

    void closeInstrumentWindow();

    // Clipboard helpers for MIDI clips (Junie)
    void copyMidiClip (te::MidiClip* clip);
    // Paste clipboard content to a specific track at a beat position
    bool pasteClipboardAt (int trackIndex, double startBeats);
    // Duplicate a specific MIDI clip right after itself
    bool duplicateMidiClip (te::MidiClip* clip);
    // Delete a specific MIDI clip
    bool deleteMidiClip (te::MidiClip* clip);
    // Check if clipboard has content (Junie)
    bool hasClipboardContent() const;

private:
    std::unique_ptr<tracktion::engine::Engine> engine;
    std::unique_ptr<tracktion::engine::Edit> edit;
    std::unique_ptr<te::SelectionManager> selectionManager;

    std::unique_ptr<EditViewState> editViewState;

    std::unique_ptr<MIDIEngine> midiEngine;
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<TrackManager> trackManager;
    std::unique_ptr<MidiListener> midiListener;

    // Map from track index to its controller listener (TrackComponent) (Junie)
    juce::HashMap<int, TrackHeaderComponent::Listener*> trackListenerMap;

    std::atomic_bool shuttingDown { false };

    juce::File currentEditFile;

    std::unique_ptr<juce::DocumentWindow> instrumentWindow_;

    int lastSavedTxn = 0;

    bool writeEditToFile (const juce::File& file);
    void markSaved();
    int currentUndoTxn() const;

    juce::File getAutosaveFile() const;
    void timerCallback() override;

    int selectedTrackIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppEngine)
};