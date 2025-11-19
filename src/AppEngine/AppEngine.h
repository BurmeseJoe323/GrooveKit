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

/**
 * @brief Manages view-specific state for the Tracktion Edit, persisted in the Edit's ValueTree.
 *
 * EditViewState wraps Tracktion Edit-specific settings that control which tracks/elements
 * are visible in the UI. State is automatically saved with the Edit and restored on load.
 *
 * Coordinates managed:
 *  - Track visibility flags (master, global, marker, chord, arranger)
 *  - Waveform display settings
 *  - View scroll position (viewX1, viewX2) and zoom (viewY)
 *
 * This class provides coordinate conversion methods for timeline rendering but does not
 * handle actual track data - that responsibility belongs to AppEngine and TrackManager.
 */
class EditViewState
{
public:
    /**
     * @brief Constructs EditViewState and binds to the Edit's ValueTree.
     *
     * @param e Reference to the Tracktion Edit.
     * @param s Reference to the SelectionManager.
     */
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

    /**
     * @brief Converts a time position to X coordinate in pixels.
     *
     * @param time Timeline position.
     * @param width Width of the view in pixels.
     * @return X coordinate in pixels.
     */
    int timeToX (t::TimePosition time, int width) const
    {
        return juce::roundToInt (((time - viewX1) * width) / (viewX2 - viewX1));
    }

    /**
     * @brief Converts an X coordinate to timeline position.
     *
     * @param x X coordinate in pixels.
     * @param width Width of the view in pixels.
     * @return Timeline position.
     */
    t::TimePosition xToTime (int x, int width) const
    {
        return t::toPosition ((viewX2 - viewX1) * (double (x) / width)) + t::toDuration (viewX1.get());
    }

    /**
     * @brief Converts a beat position to timeline time using the Edit's tempo sequence.
     *
     * @param b Beat position.
     * @return Timeline position in seconds.
     */
    t::TimePosition beatToTime (t::BeatPosition b) const
    {
        auto& ts = edit.tempoSequence;
        return ts.toTime (b);
    }

    te::Edit& edit; ///< Reference to the Tracktion Edit (not owned).
    te::SelectionManager& selectionManager; ///< Reference to the SelectionManager (not owned).

    juce::CachedValue<bool> showMasterTrack, showGlobalTrack, showMarkerTrack, showChordTrack, showArrangerTrack,
        drawWaveforms, showHeaders, showFooters, showMidiDevices, showWaveDevices; ///< View visibility flags.

    juce::CachedValue<t::TimePosition> viewX1, viewX2; ///< Horizontal view range (scroll position).
    juce::CachedValue<double> viewY; ///< Vertical view offset (zoom level).

    juce::ValueTree state; ///< Persistent state storage in Edit's ValueTree.
};

/**
 * @brief Central application engine managing audio, MIDI, tracks, and edit state.
 *
 * AppEngine is the primary coordinator and facade for GrooveKit, providing a simplified
 * interface to Tracktion Engine's complex APIs. It owns and manages:
 *  - Tracktion Engine and Edit instances
 *  - AudioEngine (playback, device configuration, MIDI routing)
 *  - MIDIEngine (MIDI clip creation and management)
 *  - TrackManager (track lifecycle, mute/solo, drum vs instrument)
 *  - MidiListener (QWERTY keyboard input)
 *  - Edit file management (save, load, autosave)
 *  - Clipboard operations (copy/paste/duplicate MIDI clips)
 *
 * Architecture:
 *  UI components should always interact through AppEngine, never directly with Tracktion Engine.
 *  AppEngine maintains application state (armed track, dirty flag, clipboard) and coordinates
 *  between subsystems (e.g., arming a track updates TrackManager and routes MIDI via AudioEngine).
 *
 * Key responsibilities:
 *  - Track creation/deletion with automatic UI notification
 *  - Transport control (play/stop/record)
 *  - Tempo and metronome management
 *  - MIDI clip clipboard operations with type validation (drum vs instrument)
 *  - Edit persistence with autosave timer
 *  - Track listener registry for UI component communication
 *
 * @note AppEngine inherits from juce::Timer for autosave functionality.
 * @note This is a singleton-like class - MainComponent owns one instance passed to all views.
 */
class AppEngine : private juce::Timer
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the AppEngine and initializes Tracktion Engine subsystems.
     *
     * Creates the Tracktion Engine, Edit, AudioEngine, MIDIEngine, TrackManager, and MidiListener.
     * Initializes audio with default settings (48kHz, 512 buffer) and loads or creates an Edit.
     */
    AppEngine();

    /**
     * @brief Destructor - shuts down subsystems and saves dirty edits.
     */
    ~AppEngine() override;

    //==============================================================================
    // Edit Management

    /**
     * @brief Creates a new untitled edit or loads the most recent edit from disk.
     *
     * Searches the user's GrooveKit folder for .tracktionedit files and loads the most recent.
     * If none found, creates a new empty edit.
     */
    void createOrLoadEdit();

    /**
     * @brief Creates a new untitled edit, discarding current unsaved changes.
     *
     * Resets the Edit to an empty state without saving. Used when user confirms discarding changes.
     */
    void newUntitledEdit();

    /**
     * @brief Saves the current edit to its existing file.
     *
     * @return true if save succeeded, false if no file path set (use saveEditAsAsync instead).
     */
    bool saveEdit();

    /**
     * @brief Opens a file chooser dialog to save the edit with a new name.
     *
     * Asynchronous operation - callback is invoked when user completes file selection.
     *
     * @param onDone Callback invoked with true if save succeeded, false if canceled/failed.
     */
    void saveEditAsAsync (std::function<void (bool success)> onDone = {});

    /**
     * @brief Opens a file chooser dialog to load an existing edit.
     *
     * Asynchronous operation - callback is invoked when user completes file selection.
     *
     * @param onDone Callback invoked with true if load succeeded, false if canceled/failed.
     */
    void openEditAsync (std::function<void (bool success)> onDone = {});

    /**
     * @brief Loads an edit from a specific file path.
     *
     * @param file The .tracktionedit file to load.
     * @return true if load succeeded, false on error.
     */
    bool loadEditFromFile (const juce::File& file);

    /**
     * @brief Returns whether the edit has unsaved changes.
     *
     * Compares current undo transaction count with last saved transaction.
     *
     * @return true if edit has been modified since last save.
     */
    bool isDirty() const noexcept;

    /**
     * @brief Returns the file path of the currently loaded edit.
     *
     * @return File reference, or invalid File if edit is untitled.
     */
    const juce::File& getCurrentEditFile() const noexcept { return currentEditFile; }

    /**
     * @brief Sets the autosave interval in minutes.
     *
     * @param minutes Autosave interval (0 to disable).
     */
    void setAutosaveMinutes (int minutes);

    /**
     * @brief Callback invoked after an edit is loaded or created.
     *
     * UI components should register this to refresh their state.
     */
    std::function<void()> onEditLoaded;

    //==============================================================================
    // Transport Control

    /**
     * @brief Starts playback of the edit.
     *
     * Delegates to AudioEngine which handles loop positioning.
     */
    void play();

    /**
     * @brief Stops playback and keeps any in-progress recordings.
     *
     * Delegates to AudioEngine which also stops all plugin notes.
     */
    void stop();

    /**
     * @brief Returns whether playback is currently active.
     *
     * @return true if playing, false otherwise.
     */
    bool isPlaying() const;

    /**
     * @brief Toggles recording state on/off.
     *
     * Automatically arms the selected track if not already armed. Delegates to AudioEngine.
     */
    void toggleRecord();

    /**
     * @brief Returns whether recording is currently active.
     *
     * @return true if recording, false otherwise.
     */
    bool isRecording() const;

    /**
     * @brief Callback invoked when recording stops.
     *
     * UI components can register this to refresh clip displays after recording.
     */
    std::function<void()> onRecordingStopped;

    //==============================================================================
    // Tempo and Metronome

    /**
     * @brief Returns the current tempo in beats per minute.
     *
     * @return BPM value from the Edit's tempo sequence.
     */
    double getBpm() const;

    /**
     * @brief Sets the tempo in beats per minute.
     *
     * Updates the Edit's tempo sequence and adjusts loop range and playhead position
     * to maintain beat positions across tempo changes.
     *
     * @param newBpm New tempo (must be > 0).
     */
    void setBpm (double newBpm);

    /**
     * @brief Callback invoked when tempo changes.
     *
     * Provides old BPM, new BPM, previous loop range, and previous playhead position
     * for UI components to maintain visual beat positions.
     */
    std::function<void(double oldBpm, double newBpm, t::TimeRange oldLoopRange, t::TimePosition oldPlayheadPos)> onBpmChanged;

    /**
     * @brief Enables or disables the metronome/click track.
     *
     * @param enabled true to enable, false to disable.
     */
    void setClickTrackEnabled (bool enabled);

    /**
     * @brief Returns whether the metronome is enabled.
     *
     * @return true if enabled, false otherwise.
     */
    bool isClickTrackEnabled() const;

    /**
     * @brief Sets whether the metronome only plays during recording.
     *
     * @param recordingOnly true for recording-only, false for always-on when enabled.
     */
    void setClickTrackRecordingOnly (bool recordingOnly);

    /**
     * @brief Returns whether the metronome is set to recording-only mode.
     *
     * @return true if recording-only, false if always-on.
     */
    bool isClickTrackRecordingOnly() const;

    //==============================================================================
    // Track Management

    /**
     * @brief Adds a new MIDI track to the edit.
     *
     * @deprecated Use addDrumTrack() or addInstrumentTrack() instead for type-specific creation.
     * @return Index of the newly created track.
     */
    int addMidiTrack();

    /**
     * @brief Adds a new drum track with drum sampler.
     *
     * Creates a track, marks it as drum type (gk_isDrum property), and initializes
     * a DrumSamplerEngineAdapter instance.
     *
     * @return Index of the newly created drum track.
     */
    int addDrumTrack();

    /**
     * @brief Adds a new instrument track.
     *
     * Creates a standard MIDI track without drum sampler.
     *
     * @return Index of the newly created instrument track.
     */
    int addInstrumentTrack();

    /**
     * @brief Returns the total number of tracks in the edit.
     *
     * @return Track count.
     */
    int getNumTracks();

    /**
     * @brief Deletes a track by index.
     *
     * @param index Track index to delete.
     */
    void deleteMidiTrack (int index);

    /**
     * @brief Returns whether a track is a drum track.
     *
     * Checks the track's gk_isDrum property set by TrackManager.
     *
     * @param index Track index.
     * @return true if drum track, false if instrument track.
     */
    bool isDrumTrack (int index) const;

    /**
     * @brief Returns the DrumSamplerEngineAdapter for a drum track.
     *
     * @param index Track index (must be a drum track).
     * @return Pointer to DrumSamplerEngineAdapter, or nullptr if not a drum track.
     */
    DrumSamplerEngineAdapter* getDrumAdapter (int index);

    //==============================================================================
    // Track State (Mute/Solo/Naming)

    /**
     * @brief Sets the mute state of a track.
     *
     * @param index Track index.
     * @param mute true to mute, false to unmute.
     */
    void setTrackMuted (int index, bool mute) { trackManager->setTrackMuted (index, mute); }

    /**
     * @brief Returns whether a track is muted.
     *
     * @param index Track index.
     * @return true if muted, false otherwise.
     */
    bool isTrackMuted (int index) const { return trackManager->isTrackMuted (index); }

    /**
     * @brief Solos a track (mutes all other tracks).
     *
     * @param index Track index to solo.
     */
    void soloTrack (int index);

    /**
     * @brief Sets the solo state of a track.
     *
     * @param index Track index.
     * @param solo true to solo, false to unsolo.
     */
    void setTrackSoloed (int index, bool solo);

    /**
     * @brief Returns whether a track is soloed.
     *
     * @param index Track index.
     * @return true if soloed, false otherwise.
     */
    bool isTrackSoloed (int index) const;

    /**
     * @brief Returns whether any track is currently soloed.
     *
     * @return true if at least one track is soloed, false otherwise.
     */
    bool anyTrackSoloed() const;

    /**
     * @brief Sets the display name of a track.
     *
     * @param trackIndex Track index.
     * @param name New name for the track.
     */
    void setTrackName (int trackIndex, const juce::String& name);

    /**
     * @brief Returns the display name of a track.
     *
     * @param trackIndex Track index.
     * @return Track name.
     */
    juce::String getTrackName (int trackIndex) const;

    //==============================================================================
    // Track Arming (for Recording)

    /**
     * @brief Arms a track for MIDI recording.
     *
     * Disarms all other tracks and routes all MIDI inputs to the specified track.
     *
     * @param index Track index to arm (-1 to disarm all).
     */
    void setArmedTrack (int index);

    /**
     * @brief Returns the index of the currently armed track.
     *
     * @return Track index, or -1 if no track is armed.
     */
    int getArmedTrackIndex() const;

    /**
     * @brief Returns a pointer to the currently armed track.
     *
     * @return Pointer to te::AudioTrack, or nullptr if no track is armed.
     */
    te::AudioTrack* getArmedTrack();

    /**
     * @brief Callback invoked when the armed track changes.
     *
     * UI components can register this to update armed track indicators.
     */
    std::function<void()> onArmedTrackChanged;

    //==============================================================================
    // MIDI Clip Management

    /**
     * @brief Adds an empty MIDI clip to a track at the current playhead position.
     *
     * @param trackIndex Track index.
     * @return true if clip was created, false on error.
     */
    bool addMidiClipToTrack (int trackIndex);

    /**
     * @brief Adds an empty MIDI clip to a track at a specific position and length.
     *
     * @param trackIndex Track index.
     * @param start Start position in timeline.
     * @param length Duration in beats.
     * @return true if clip was created, false on error.
     */
    bool addMidiClipToTrackAt (int trackIndex, t::TimePosition start, t::BeatDuration length);

    /**
     * @brief Returns the first MIDI clip on a track (deprecated).
     *
     * @deprecated Use getMidiClipsFromTrack() to get all clips.
     * @param trackIndex Track index.
     * @return Pointer to first MidiClip, or nullptr if none.
     */
    te::MidiClip* getMidiClipFromTrack (int trackIndex);

    /**
     * @brief Returns all MIDI clips on a track.
     *
     * @param trackIndex Track index.
     * @return Array of MidiClip pointers.
     */
    juce::Array<te::MidiClip*> getMidiClipsFromTrack (int trackIndex);

    //==============================================================================
    // Clipboard Operations

    /**
     * @brief Copies a MIDI clip to the internal clipboard.
     *
     * Stores clip state (MIDI notes, position, length) and track type (drum vs instrument)
     * for paste validation.
     *
     * @param clip Pointer to the MidiClip to copy.
     */
    void copyMidiClip (te::MidiClip* clip);

    /**
     * @brief Pastes clipboard content to a track at a specific beat position.
     *
     * Creates a new clip with copied MIDI notes. Validates track type compatibility
     * (drum clips can only paste to drum tracks, instrument clips to instrument tracks).
     *
     * @param trackIndex Target track index.
     * @param startBeats Start position in beats.
     * @return true if paste succeeded, false if no clipboard data or type mismatch.
     */
    bool pasteClipboardAt (int trackIndex, double startBeats);

    /**
     * @brief Duplicates a MIDI clip immediately after itself on the same track.
     *
     * @param clip Pointer to the MidiClip to duplicate.
     * @return true if duplication succeeded, false on error.
     */
    bool duplicateMidiClip (te::MidiClip* clip);

    /**
     * @brief Deletes a MIDI clip from its track.
     *
     * @param clip Pointer to the MidiClip to delete.
     * @return true if deletion succeeded, false on error.
     */
    bool deleteMidiClip (te::MidiClip* clip);

    /**
     * @brief Returns whether the clipboard has content.
     *
     * @return true if a clip has been copied, false otherwise.
     */
    bool hasClipboardContent() const;

    /**
     * @brief Returns whether clipboard content can be pasted to a specific track.
     *
     * Validates track type compatibility (drum vs instrument).
     *
     * @param trackIndex Target track index.
     * @return true if paste is allowed, false if type mismatch.
     */
    bool canPasteToTrack (int trackIndex) const;

    /**
     * @brief Returns the length of the clipboard clip in beats.
     *
     * Used for overlap detection when pasting.
     *
     * @return Clip length in beats, or 0.0 if clipboard is empty.
     */
    double getClipboardClipLengthBeats() const { return lastCopiedClipLengthBeats; }

    //==============================================================================
    // Track Listener Registry

    /**
     * @brief Registers a listener for track header button events.
     *
     * TrackComponent registers itself to receive mute/solo/arm/delete notifications.
     *
     * @param index Track index.
     * @param l Pointer to the listener (TrackComponent).
     */
    void registerTrackListener (int index, TrackHeaderComponent::Listener* l);

    /**
     * @brief Unregisters a track listener.
     *
     * @param index Track index.
     * @param l Pointer to the listener to remove.
     */
    void unregisterTrackListener (int index, TrackHeaderComponent::Listener* l);

    /**
     * @brief Returns the registered listener for a track.
     *
     * @param index Track index.
     * @return Pointer to the listener, or nullptr if none registered.
     */
    [[nodiscard]] TrackHeaderComponent::Listener* getTrackListener (int index) const;

    //==============================================================================
    // Instrument/Plugin Management

    /**
     * @brief Creates a FourOsc audition patch on a track.
     *
     * @param trackIndex Track index to add the plugin to.
     */
    void makeFourOscAuditionPatch (int trackIndex);

    /**
     * @brief Opens the instrument editor window for a track.
     *
     * @param trackIndex Track index.
     */
    void openInstrumentEditor (int trackIndex);

    /**
     * @brief Closes the currently open instrument editor window.
     */
    void closeInstrumentWindow();

    //==============================================================================
    // Audio Device Configuration

    /**
     * @brief Sets the audio output device by name.
     *
     * Delegates to AudioEngine.
     *
     * @param name Output device name.
     * @return true if successful, false on error.
     */
    bool setOutputDevice (const juce::String& name)        { return audioEngine->setOutputDeviceByName (name); }

    /**
     * @brief Reverts to the system's default audio output device.
     *
     * @return true if successful, false on error.
     */
    bool setDefaultOutputDevice()                          { return audioEngine->setDefaultOutputDevice(); }

    /**
     * @brief Lists all available audio output devices.
     *
     * @return StringArray of device names.
     */
    juce::StringArray listOutputDevices()            const { return audioEngine->listOutputDevices(); }

    /**
     * @brief Returns the name of the current audio output device.
     *
     * @return Device name, or empty string if none.
     */
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
     * @return StringArray of MIDI device names.
     */
    juce::StringArray listMidiInputDevices()         const { return audioEngine->listMidiInputDevices(); }

    /**
     * @brief Returns list of currently enabled MIDI input devices.
     *
     * @return StringArray of enabled device names.
     */
    juce::StringArray getEnabledMidiDevices()        const { return audioEngine->getEnabledMidiDevices(); }

    /**
     * @brief Checks if a MIDI input device is enabled.
     *
     * @param deviceName Name of the MIDI device.
     * @return true if enabled, false otherwise.
     */
    bool isMidiDeviceEnabled (const juce::String& deviceName) const { return audioEngine->isMidiDeviceEnabled (deviceName); }

    /**
     * @brief Enables or disables a MIDI input device.
     *
     * @param deviceName Name of the MIDI device.
     * @param enabled true to enable, false to disable.
     */
    void setMidiDeviceEnabled (const juce::String& deviceName, bool enabled) { audioEngine->setMidiDeviceEnabled (deviceName, enabled); }

    //==============================================================================
    // Subsystem Access

    /**
     * @brief Returns reference to the TrackManager.
     *
     * @return TrackManager reference.
     */
    TrackManager& getTrackManager()       { return *trackManager; }

    /**
     * @brief Returns pointer to the TrackManager.
     *
     * @return TrackManager pointer.
     */
    TrackManager* getTrackManagerPtr()    { return trackManager.get(); }

    /**
     * @brief Returns reference to the AudioEngine.
     *
     * @return AudioEngine reference.
     */
    AudioEngine& getAudioEngine();

    /**
     * @brief Returns reference to the MIDIEngine.
     *
     * @return MIDIEngine reference.
     */
    MIDIEngine& getMidiEngine();

    /**
     * @brief Returns reference to the MidiListener.
     *
     * @return MidiListener reference.
     */
    MidiListener& getMidiListener() const { return *midiListener; }

    /**
     * @brief Returns reference to the JUCE AudioProcessorValueTreeState.
     *
     * Used for plugin parameter management.
     *
     * @return APVTS reference.
     */
    juce::AudioProcessorValueTreeState& getAPVTS();

    /**
     * @brief Returns reference to the EditViewState.
     *
     * @return EditViewState reference.
     */
    EditViewState& getEditViewState();

    /**
     * @brief Returns reference to the Tracktion Edit.
     *
     * @return Edit reference.
     */
    te::Edit& getEdit();

private:
    //==============================================================================
    // Internal Methods

    /**
     * @brief Writes the edit to a file on disk.
     *
     * @param file File path to write to.
     * @return true if successful, false on error.
     */
    bool writeEditToFile (const juce::File& file);

    /**
     * @brief Marks the edit as saved (resets dirty flag).
     */
    void markSaved();

    /**
     * @brief Returns the current undo transaction number.
     *
     * @return Transaction number.
     */
    int currentUndoTxn() const;

    /**
     * @brief Returns the autosave file path.
     *
     * @return File reference for autosave location.
     */
    juce::File getAutosaveFile() const;

    /**
     * @brief Timer callback for autosave functionality.
     */
    void timerCallback() override;

    //==============================================================================
    // Member Variables

    std::unique_ptr<tracktion::engine::Engine> engine; ///< Tracktion Engine instance (owned).
    std::unique_ptr<tracktion::engine::Edit> edit; ///< Tracktion Edit instance (owned).
    std::unique_ptr<te::SelectionManager> selectionManager; ///< Selection manager for the Edit (owned).

    std::unique_ptr<EditViewState> editViewState; ///< View state manager (owned).

    std::unique_ptr<MIDIEngine> midiEngine; ///< MIDI clip management (owned).
    std::unique_ptr<AudioEngine> audioEngine; ///< Audio playback and device management (owned).
    std::unique_ptr<TrackManager> trackManager; ///< Track lifecycle and type management (owned).
    std::unique_ptr<MidiListener> midiListener; ///< QWERTY keyboard MIDI input (owned).

    juce::HashMap<int, TrackHeaderComponent::Listener*> trackListenerMap; ///< Track index to UI listener map (not owned).

    std::atomic_bool shuttingDown { false }; ///< Flag to prevent operations during destruction.

    juce::File currentEditFile; ///< Path to the currently loaded edit file.

    std::unique_ptr<juce::DocumentWindow> instrumentWindow_; ///< Plugin editor window (owned).

    int lastSavedTxn = 0; ///< Last undo transaction number when edit was saved.

    int selectedTrackIndex = -1; ///< Currently armed track index (-1 if none).

    bool lastCopiedClipWasDrum = false; ///< Type of last copied clip for paste validation.
    bool hasClipboardTypeInfo = false; ///< Whether clipboard has valid type information.
    double lastCopiedClipLengthBeats = 0.0; ///< Length of clipboard clip in beats.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppEngine)
};
