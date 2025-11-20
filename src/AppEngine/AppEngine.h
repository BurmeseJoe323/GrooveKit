#pragma once

#include "../AudioEngine/AudioEngine.h"
#include "../MIDIEngine/MIDIEngine.h"
#include "../PluginManager/PluginManager.h"
#include "../UI/TrackView/TrackHeaderComponent.h"
#include "TrackManager.h"
#include "MidiListener.h"
#include "MidiRecorder.h"
#include <tracktion_engine/tracktion_engine.h>
struct MidiListenerKeyAdapter;
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

    void initialise();

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
    void wireAllMidiInputsToTrack (tracktion::engine::AudioTrack& track);

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

    // Track naming (Written by Claude Code)
    void setTrackName (int trackIndex, const juce::String& name);
    juce::String getTrackName (int trackIndex) const;
    std::function<void(int)> onInstrumentLabelChanged;

    //==============================================================================
    // Transport and Tempo

    double getBpm() const;
    void setBpm (double newBpm);

    //==============================================================================
    // Metronome Control

    /**
     * @brief Enables or disables the metronome/click track.
     *
     * @param enabled True to enable, false to disable.
     */
    void setClickTrackEnabled (bool enabled);

    /**
     * @brief Returns whether the metronome/click track is enabled.
     *
     * @return True if enabled, false otherwise.
     */
    bool isClickTrackEnabled() const;

    /**
     * @brief Sets whether the metronome plays only during recording.
     *
     * @param recordingOnly True to play only during recording, false to play always.
     */
    void setClickTrackRecordingOnly (bool recordingOnly);

    /**
     * @brief Returns whether the metronome plays only during recording.
     *
     * @return True if recording-only mode is enabled, false otherwise.
     */
    bool isClickTrackRecordingOnly() const;

    /**
     * @brief Enables verbose MIDI input device state logging for debugging.
     *
     * When enabled, logs the state of all MIDI input devices including:
     *  - Device enabled status
     *  - Recording enabled status
     *  - Monitor mode settings
     *  - Target track routing
     *
     * Useful for debugging recording issues. Call this before starting a
     * recording session to see diagnostic information.
     *
     * @param enable True to enable logging, false to disable.
     */
    void setMidiEventLoggingEnabled(bool enable);

    //==============================================================================
    // Track Arming and Recording Control

    /**
     * @brief Arms a track for recording by routing MIDI inputs to it.
     *
     * When a track is armed:
     *  - All physical MIDI input devices are routed to the track
     *  - The track is pre-armed for recording (setRecordingEnabled called)
     *  - Live MIDI monitoring is enabled (automatic monitor mode)
     *
     * Calling this with the same index multiple times is a no-op. To disarm a track,
     * arm a different track or pass -1.
     *
     * @param index The 0-based track index to arm, or -1 to disarm all tracks.
     */
    void setArmedTrack (int index);

    /**
     * @brief Returns the index of the currently armed track.
     *
     * @return The armed track index, or -1 if no track is armed.
     */
    int getArmedTrackIndex() const;

    /**
     * @brief Returns a pointer to the currently armed track.
     *
     * @return Pointer to the armed AudioTrack, or nullptr if no track is armed.
     */
    te::AudioTrack* getArmedTrack();

    /**
     * @brief Callback invoked when the armed track changes.
     *
     * UI components can register this callback to update visual state
     * when track arming changes.
     */
    std::function<void()> onArmedTrackChanged;

    /**
     * @brief Toggles the recording state.
     *
     * If not recording:
     *  - Starts recording on the currently armed track (if any)
     *  - Automatically starts transport if not already playing
     *  - Positions at loop start if looping is enabled
     *
     * If recording:
     *  - Stops recording and keeps recorded clips (does not discard)
     *  - Calls onRecordingStopped callback asynchronously after clip finalization
     *
     * This method validates that a track is armed before allowing recording to start.
     */
    void toggleRecord();

    /**
     * @brief Returns whether recording is currently active.
     *
     * @return True if recording, false otherwise.
     */
    bool isRecording() const;

    /**
     * @brief Returns the current recording preview clip bounds.
     *
     * Used for real-time visual feedback during recording. Returns the time range
     * from when the first MIDI note was captured to the current transport position.
     * Returns an empty range if not recording or no notes captured yet.
     *
     * @return TimeRange representing the preview clip bounds, or empty if no notes recorded.
     */
    tracktion::TimeRange getRecordingPreviewBounds() const;

    /**
     * @brief Callback invoked after recording stops and clips are finalized.
     *
     * This is called asynchronously (via MessageManager::callAsync) to allow
     * Tracktion time to create and finalize clips before UI updates.
     *
     * UI components can register this callback to refresh clip displays after recording.
     */
    std::function<void()> onRecordingStopped;

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

    /**
     * @brief Returns available buffer sizes supported by the current audio device.
     *
     * @return Array of buffer sizes in samples.
     */
    juce::Array<int> getAvailableBufferSizes()       const { return audioEngine->getAvailableBufferSizes(); }

    /**
     * @brief Returns available sample rates supported by the current audio device.
     *
     * @return Array of sample rates in Hz.
     */
    juce::Array<double> getAvailableSampleRates()    const { return audioEngine->getAvailableSampleRates(); }

    /**
     * @brief Returns the current audio buffer size.
     *
     * @return Buffer size in samples.
     */
    int getCurrentBufferSize()                       const { return audioEngine->getCurrentBufferSize(); }

    /**
     * @brief Returns the current audio sample rate.
     *
     * @return Sample rate in Hz.
     */
    double getCurrentSampleRate()                    const { return audioEngine->getCurrentSampleRate(); }

    /**
     * @brief Sets the audio buffer size.
     *
     * @param bufferSize Buffer size in samples.
     * @return true if successful, false on error.
     */
    bool setBufferSize (int bufferSize)                    { return audioEngine->setBufferSize (bufferSize); }

    /**
     * @brief Sets the audio sample rate.
     *
     * @param sampleRate Sample rate in Hz.
     * @return true if successful, false on error.
     */
    bool setSampleRate (double sampleRate)                 { return audioEngine->setSampleRate (sampleRate); }

    //==============================================================================
    // MIDI Input Device Management

    /**
     * @brief Lists all available MIDI input devices.
     *
     * Tracktion automatically manages MIDI device enabling, so this is for display purposes only.
     *
     * @return StringArray of MIDI device names.
     */
    juce::StringArray listMidiInputDevices()         const { return audioEngine->listMidiInputDevices(); }

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
    /** Opens a file chooser, imports a MIDI file, and drops it on the given track.
        @param trackIndex Index of the target track
        @param destStart  Where to place the clip (usually current transport pos or bar boundary)
    */
    void importMidiClipViaChooser (int trackIndex,
                               t::TimePosition destStart,
                               std::function<void()> onSuccess = {});
    // Check if clipboard has content (Junie)
    bool hasClipboardContent() const;
    // Check if clipboard content can be pasted to a specific track (Junie)
    bool canPasteToTrack (int trackIndex) const;
    // Get clipboard clip length in beats (Written by Claude Code)
    double getClipboardClipLengthBeats() const { return lastCopiedClipLengthBeats; }

    void showInstrumentChooser (int trackIndex);

    void onFxInsertSlotClicked (int trackIndex,
                            int slotIndex,
                            std::function<void (const juce::String&)> onSlotLabelChange);

    void showFxInsertMenu (int trackIndex,
                           int slotIndex,
                           std::function<void (const juce::String&)> onSlotLabelChange);

    juce::String getInstrumentLabelForTrack (int trackIndex) const;
    juce::String getInsertSlotLabel       (int trackIndex, int slotIndex) const;

    PluginManager& getPluginManager() { return *pluginManager; }

    bool exportAudio (const juce::File& destFile);


private:
    std::unique_ptr<tracktion::engine::Engine> engine;
    std::unique_ptr<tracktion::engine::Edit> edit;
    std::unique_ptr<te::SelectionManager> selectionManager;

    std::unique_ptr<EditViewState> editViewState;

    std::unique_ptr<MIDIEngine> midiEngine;
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<TrackManager> trackManager;
    std::unique_ptr<PluginManager> pluginManager;
    std::unique_ptr<MidiListener> midiListener;
    std::unique_ptr<MidiRecorder> midiRecorder;
    std::unique_ptr<MidiListenerKeyAdapter> qwertyForwarder_;

    // Map from track index to its controller listener (TrackComponent) (Junie)
    juce::HashMap<int, TrackHeaderComponent::Listener*> trackListenerMap;

    std::atomic_bool shuttingDown { false };

    juce::File currentEditFile;

    std::unique_ptr<juce::DocumentWindow> instrumentWindow_;

    bool startedTransportForEditor_ = false;

    int lastSavedTxn = 0;



    bool writeEditToFile (const juce::File& file);
    void markSaved();
    int currentUndoTxn() const;

    juce::File getAutosaveFile() const;
    void timerCallback() override;


    int selectedTrackIndex = -1;

    // Track the type of the last copied clip for paste validation (Junie)
    bool lastCopiedClipWasDrum = false;
    bool hasClipboardTypeInfo = false;
    double lastCopiedClipLengthBeats = 0.0; // Length in beats (Written by Claude Code)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppEngine)
};