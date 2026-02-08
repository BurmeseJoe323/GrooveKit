#include "AppEngine.h"

#include "../DrumSamplerEngine/DefaultSampleLibrary.h"
#include "../PluginManager/PluginEditorWindow.h"
#include "../UI/Plugins/FourOsc/FourOscGUI.h"
#include "../PluginManager/PluginEditorWindow.h"
#include "../UI/Plugins/Synthesizer/MorphSynthRegistration.h"
#include "../UI/Plugins/Synthesizer/MorphSynthView.h"
#include "../UI/Plugins/Synthesizer/MorphSynthWindow.h"
#include "GrooveKitUIBehaviour.h"
#include "TrackManager.h"
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;
namespace t = tracktion;
using namespace std::literals;
using namespace t::literals;

namespace GKIDs {
    static const juce::Identifier isDrumClip ("gk_isDrumClip");
}

struct MidiListenerKeyAdapter : public juce::KeyListener
{
    explicit MidiListenerKeyAdapter (MidiListener& target) : ml (target) {}

    bool keyPressed (const juce::KeyPress& key, juce::Component* /*origin*/) override
    {
        // Let MidiListener handle octave keys & note-key recognition
        bool handled = ml.handleKeyPress (key);

        // Drive its state machine for note-on based on currently-down keys
        bool handledState = ml.handleKeyStateChanged (true);

        return handled || handledState; // swallow if we did anything
    }

    bool keyStateChanged (bool isDown, juce::Component* /*origin*/) override
    {
        // Release notes (and handle any new downs if needed)
        return ml.handleKeyStateChanged (isDown);
    }

private:
    MidiListener& ml;
};

namespace
{
    const juce::StringArray applePluginNameBlacklist
    {
        "AUGraphicEQ",
        "AUNBandEQ",
        "AUParametricEQ",
        "Apple: AUMatrixReverb",
        "Apple: AUDynamicsProcessor",
        // add any problematic plugins
    };

    bool isAppleBuiltInPlugin (const juce::PluginDescription& d)
    {
        const auto fmt  = d.pluginFormatName.toLowerCase();
        const auto manu = d.manufacturerName.toLowerCase();
        const auto name = d.name.toLowerCase();

        if (! fmt.contains ("audiounit"))
            return false;

        if (! manu.contains ("apple"))
            return false;

        for (auto& n : applePluginNameBlacklist)
            if (name == n.toLowerCase())
                return true;

        return true;
    }
}

AppEngine::AppEngine()
{
    engine = std::make_unique<te::Engine> (
        "GrooveKitEngine",
        std::make_unique<GrooveKitUIBehaviour>(),
        nullptr
    );

    registerMorphSynthCompat(*engine);

    createOrLoadEdit();

    midiEngine = std::make_unique<MIDIEngine> (*edit);
    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    trackManager = std::make_unique<TrackManager> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);
    midiListener = std::make_unique<MidiListener> (this);
    midiRecorder = std::make_unique<MidiRecorder> (*engine);

    qwertyForwarder_ = std::make_unique<MidiListenerKeyAdapter>(*midiListener);

    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    audioEngine->initialiseDefaults (48000.0, 512);

    // Setup MIDI input devices using Tracktion's InputDevice system
    // CRITICAL: This must be called BEFORE restartPlayback() to ensure proper MIDI routing
    audioEngine->setupMidiInputDevices(*edit);

    // Now restart playback with MIDI devices properly enabled
    edit->restartPlayback();
}

AppEngine::~AppEngine()
{
    // Signal shutdown early so UI listeners don't touch the registry while we tear down
    shuttingDown = true;
    // Clear listener map defensively to release any dangling pointers
    trackListenerMap.clear();
}

// Listener registry methods (Junie)
void AppEngine::registerTrackListener (const int index, TrackHeaderComponent::Listener* l)
{
    // Guard against invalid indices to avoid JUCE HashMap hash assertions
    if (shuttingDown || index < 0 || l == nullptr)
        return;
    trackListenerMap.set (index, l);
}

void AppEngine::unregisterTrackListener (const int index, TrackHeaderComponent::Listener* l)
{
    if (shuttingDown || index < 0)
        return;

    if (trackListenerMap.contains (index))
    {
        if (trackListenerMap[index] == l)
            trackListenerMap.remove (index);
    }
}

TrackHeaderComponent::Listener* AppEngine::getTrackListener (const int index) const
{
    if (shuttingDown || index < 0)
        return nullptr;

    if (trackListenerMap.contains (index))
        return trackListenerMap[index];
    return nullptr;
}

//==============================================================================
// Edit Management

void AppEngine::createOrLoadEdit()
{
    auto baseDir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                       .getChildFile ("GrooveKit");
    baseDir.createDirectory();

    currentEditFile = "";
    edit = te::createEmptyEdit (*engine, baseDir.getNonexistentChildFile ("Untitled", ".tracktionedit"));

    for (auto* t : te::getAudioTracks (*edit))
        edit->deleteTrack (t);

    edit->editFileRetriever = [] { return juce::File {}; };
    edit->playInStopEnabled = true;
    edit->clickTrackEmphasiseBars = true;  // Emphasize downbeats with different click sample

    markSaved();
    // NOTE: restartPlayback() moved to AppEngine constructor after MIDI setup
}

void AppEngine::newUntitledEdit()
{
    closeInstrumentWindow();

    audioEngine.reset();

    auto baseDir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                       .getChildFile ("GrooveKit");
    baseDir.createDirectory();

    auto placeholder = baseDir.getNonexistentChildFile ("Untitled", ".tracktionedit", false);

    edit = te::createEmptyEdit (*engine, placeholder);

    currentEditFile = juce::File();

    edit->editFileRetriever = [placeholder] { return placeholder; };
    edit->playInStopEnabled = true;
    edit->clickTrackEmphasiseBars = true;  // Emphasize downbeats with different click sample

    for (auto* t : te::getAudioTracks (*edit))
        edit->deleteTrack (t);

    midiEngine = std::make_unique<MIDIEngine> (*edit);
    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    trackManager = std::make_unique<TrackManager> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);
    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    if (pluginManager)
        trackManager->setPluginManager (pluginManager.get());

    markSaved();

    audioEngine->initialiseDefaults (48000.0, 512);
    audioEngine->setupMidiInputDevices(*edit);

    if (onEditLoaded)
        onEditLoaded();

    edit->restartPlayback();
}

//==============================================================================
// Track Arming and Recording Control

void AppEngine::setArmedTrack (int index)
{
    if (selectedTrackIndex == index)
        return;

    selectedTrackIndex = index;

    // Route MIDI to armed track, or clear routing if disarming
    if (index >= 0)
    {
        audioEngine->routeMidiToTrack(*edit, index);
    }
    else
    {
        // Disarm: clear MIDI routing from all devices by setting target to invalid ID
        for (auto* instance : edit->getAllInputDevices())
        {
            if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice)
            {
                // Clear target by setting to an invalid EditItemID
                [[maybe_unused]] auto result = instance->setTarget(te::EditItemID(), false, nullptr, 0);
                // Disable recording on all tracks for this device
                for (auto* track : te::getAudioTracks(*edit))
                    instance->setRecordingEnabled(track->itemID, false);
            }
        }
        juce::Logger::writeToLog("[AppEngine] Cleared MIDI routing (track disarmed)");
    }

    if (onArmedTrackChanged)
        onArmedTrackChanged();

    if (auto* t = getArmedTrack())
        wireAllMidiInputsToTrack (*t);
}

te::AudioTrack* AppEngine::getArmedTrack ()
{
    return getTrackManager().getTrack (selectedTrackIndex);
}

int AppEngine::getArmedTrackIndex () const
{
    return selectedTrackIndex;
}

void AppEngine::toggleRecord()
{
    juce::Logger::writeToLog("[AppEngine] toggleRecord() called");

    bool wasRecording = isRecording();

    if (!wasRecording)
    {
        // Start recording
        if (selectedTrackIndex < 0)
        {
            juce::Logger::writeToLog("[AppEngine] Cannot start recording: no track armed");
            return;
        }

        juce::Logger::writeToLog("[AppEngine] Starting recording on track " + juce::String(selectedTrackIndex));

        // Start recording with both hardware MIDI and QWERTY keyboard
        midiRecorder->startRecording(*edit, selectedTrackIndex,
                                     &midiListener->getMidiKeyboardState());
    }
    else
    {
        // Stop recording
        juce::Logger::writeToLog("[AppEngine] Stopping recording");

        bool clipCreated = midiRecorder->stopRecording(*edit);

        if (clipCreated)
        {
            juce::Logger::writeToLog("[AppEngine] Recording stopped - clip created successfully");

            // Notify listeners that recording stopped
            if (onRecordingStopped)
                onRecordingStopped();
        }
        else
        {
            juce::Logger::writeToLog("[AppEngine] Recording stopped - no clip created (no MIDI captured)");
        }
    }
}

bool AppEngine::isRecording() const
{
    return midiRecorder && midiRecorder->isRecording();
}

tracktion::TimeRange AppEngine::getRecordingPreviewBounds() const
{
    if (midiRecorder)
        return midiRecorder->getPreviewClipBounds();

    return tracktion::TimeRange();
}

//==============================================================================
// Transport Control

void AppEngine::play() { audioEngine->play(); }

void AppEngine::stop()
{
    // Stop recording if active
    if (isRecording())
    {
        juce::Logger::writeToLog("[AppEngine] Stopping recording due to transport stop");
        midiRecorder->stopRecording(*edit);

        if (onRecordingStopped)
            onRecordingStopped();
    }

    audioEngine->stop();
}

bool AppEngine::isPlaying() const { return audioEngine->isPlaying(); }

void AppEngine::deleteMidiTrack (int index)
{
    if (index == selectedTrackIndex)
        selectedTrackIndex = -1;

    trackManager->deleteTrack (index);
}

bool AppEngine::addMidiClipToTrack (int trackIndex)
{
    if (! midiEngine)
        return false;
    return midiEngine->addMidiClipToTrack (trackIndex);
}

static te::InputDeviceInstance* findInstanceForDevice (te::Edit& edit, const te::InputDevice& dev)
{
    // getAllInputDevices() -> juce::Array<InputDeviceInstance*>
    auto instances = edit.getAllInputDevices();

    for (auto* inst : instances)
    {
        if (inst == nullptr)
            continue;

        if (&inst->getInputDevice() == &dev)
            return inst;
    }

    return nullptr;
}

void AppEngine::wireAllMidiInputsToTrack (te::AudioTrack& track)
{
    if (!engine || !edit)
        return;

    auto& dm  = engine->getDeviceManager();     // confirmed on your version
    auto& um  = edit->getUndoManager();
    auto targetID = track.itemID;               // each track has an EditItemID

    // 1) Enable all MIDI input devices globally
    for (const auto& midiPtr : dm.getMidiInDevices())     // std::vector<std::shared_ptr<MidiInputDevice>>
    {
        if (midiPtr)
            midiPtr->setEnabled (true);
    }

    // 2) Route each device’s InputDeviceInstance to this track
    for (const auto& midiPtr : dm.getMidiInDevices())
    {
        if (!midiPtr)
            continue;

        auto& dev = *midiPtr;

        if (auto* inst = findInstanceForDevice (*edit, dev))
        {
            auto result = inst->setTarget (targetID, /*moveToTrack*/ true, &um, /*index*/ 0);
            if (!result)
            {
                DBG ("[MIDI Wire] Failed to setTarget for " << dev.getName() << ": " << result.error());
                continue;
            }

            inst->setRecordingEnabled (targetID, true);
            DBG ("[MIDI Wire] Connected " << dev.getName() << " -> " << track.getName());
        }
    }

    // Make sure monitoring graph exists even when transport is stopped
    edit->getTransport().ensureContextAllocated();
}


te::MidiClip* AppEngine::getMidiClipFromTrack (int trackIndex)
{
    if (! midiEngine)
        return nullptr;
    return midiEngine->getMidiClipFromTrack (trackIndex);
}

juce::Array<te::MidiClip*> AppEngine::getMidiClipsFromTrack (int trackIndex)
{
    if (! midiEngine)
        return {};
    return midiEngine->getMidiClipsFromTrack (trackIndex);
}

int AppEngine::getNumTracks() { return trackManager ? trackManager->getNumTracks() : 0; }

EditViewState& AppEngine::getEditViewState() { return *editViewState; }

te::Edit& AppEngine::getEdit() { return *edit; }

bool AppEngine::isDrumTrack (int i) const { return trackManager ? trackManager->isDrumTrack (i) : false; }

DrumSamplerEngineAdapter* AppEngine::getDrumAdapter (int i) { return trackManager ? trackManager->getDrumAdapter (i) : nullptr; }

int AppEngine::addDrumTrack()
{
    jassert (trackManager != nullptr);
    return trackManager->addDrumTrack();
}

int AppEngine::addInstrumentTrack()
{
    jassert (trackManager != nullptr);
    const int idx = trackManager->addInstrumentTrack();

    // Arm/select the new track so MIDI gets routed
    // setArmedTrack (idx);

    return idx;
}


void AppEngine::soloTrack (int i) { trackManager->soloTrack (i); }
void AppEngine::setTrackSoloed (int i, bool s) { trackManager->setTrackSoloed (i, s); }
bool AppEngine::isTrackSoloed (int i) const { return trackManager->isTrackSoloed (i); }
bool AppEngine::anyTrackSoloed() const { return trackManager->anyTrackSoloed(); }

// Track naming implementation (Written by Claude Code)
void AppEngine::setTrackName (int trackIndex, const juce::String& name)
{
    auto audioTracks = te::getAudioTracks (*edit);
    if (trackIndex >= 0 && trackIndex < audioTracks.size())
    {
        auto* track = audioTracks[trackIndex];
        track->setName (name);
    }
}

juce::String AppEngine::getTrackName (int trackIndex) const
{
    auto audioTracks = te::getAudioTracks (*edit);
    if (trackIndex >= 0 && trackIndex < audioTracks.size())
    {
        auto* track = audioTracks[trackIndex];
        return track->getName();
    }
    return {};
}

double AppEngine::getBpm () const { return edit->tempoSequence.getTempo (0)->getBpm(); }

void AppEngine::setBpm (double newBpm)
{
    // Capture old state before changing BPM
    const double oldBpm = getBpm();
    const auto oldLoopRange = edit->getTransport().getLoopRange();
    const auto oldPlayheadPos = edit->getTransport().getPosition();

    // GrooveKit does not have tempo changes, so just get the first one
    edit->tempoSequence.getTempo (0)->setBpm (newBpm);

    // Notify listeners of BPM change with original values
    // (Tracktion may have already adjusted loop range/playhead by this point)
    if (onBpmChanged)
        onBpmChanged(oldBpm, newBpm, oldLoopRange, oldPlayheadPos);
}

void AppEngine::initialise()
{
    // --- App support dir (for caches, dead-mans file, etc.) ---
   #if JUCE_MAC
    auto appSupport = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                        .getChildFile("GrooveKit");
   #else
    auto appSupport = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                        .getChildFile("GrooveKit");
   #endif
    appSupport.createDirectory();

    // --- Your app-level plugin manager (keep as-is) ---
    PluginManager::Settings pmSettings;
    pmSettings.appDataDir      = appSupport;
    pmSettings.scanAudioUnits  = true;
    pmSettings.scanVST3        = true;

    pluginManager = std::make_unique<PluginManager>(*edit, pmSettings);
    trackManager->setPluginManager(pluginManager.get());

    // --- NEW: Initialise Tracktion Engine's PluginManager too ---
    auto& tePM = engine->getPluginManager();

    // Add formats we intend to host (guarded by JUCE host flags)
   #if JUCE_PLUGINHOST_AU
    tePM.pluginFormatManager.addFormat(std::make_unique<juce::AudioUnitPluginFormat>());
  #endif
  #if JUCE_PLUGINHOST_VST3
    tePM.pluginFormatManager.addFormat(std::make_unique<juce::VST3PluginFormat>());
  #endif

    // Do a quick blocking scan once so TE's knownPluginList is populated.
    // ExternalPlugin resolves/instantiates using *this* list+formats.
    {
        juce::FileSearchPath searchPaths;
       #if JUCE_MAC
        // AU locations (Components) — TE will use these only for AU formats
        searchPaths.add(juce::File("/Library/Audio/Plug-Ins/Components"));
        searchPaths.add(juce::File("~/Library/Audio/Plug-Ins/Components"));

        // VST3 locations
        searchPaths.add(juce::File("/Library/Audio/Plug-Ins/VST3"));
        searchPaths.add(juce::File("~/Library/Audio/Plug-Ins/VST3"));
       #endif

        static juce::File teDeadMans = appSupport.getChildFile("TE_ScanDeadMans.txt");

        for (int i = 0; i < tePM.pluginFormatManager.getNumFormats(); ++i)
        {
            auto* fmt = tePM.pluginFormatManager.getFormat(i);
            if (!fmt) continue;

            DBG("[TE-PM] Scan started for format: " + fmt->getName()
                + " paths: " + searchPaths.toString());

            juce::PluginDirectoryScanner scanner(tePM.knownPluginList,
                                                 *fmt,
                                                 searchPaths,
                                                 /*recursive*/ true,
                                                 teDeadMans,
                                                 /*allowAsync*/ false);

            juce::String err;
            while (scanner.scanNextFile(true, err))
                ; // pump scan

            if (err.isNotEmpty())
                DBG("[TE-PM] Scanner reported: " + err);
        }

        DBG("[TE-PM] Scan finished. TE known types: "
            + juce::String(tePM.knownPluginList.getNumTypes()));
    }

    // You can still keep your async scan/UI for your own list
    pluginManager->scanForPluginsAsync();

    // --- Ensure playback graph exists (useful for live monitoring) ---
    edit->getTransport().ensureContextAllocated();
}

// Metronome/Click Track controls
void AppEngine::setClickTrackEnabled (bool enabled)
{
    edit->clickTrackEnabled = enabled;
}

bool AppEngine::isClickTrackEnabled() const
{
    return edit->clickTrackEnabled;
}

void AppEngine::setClickTrackRecordingOnly (bool recordingOnly)
{
    edit->clickTrackRecordingOnly = recordingOnly;
}

bool AppEngine::isClickTrackRecordingOnly() const
{
    return edit->clickTrackRecordingOnly;
}

void AppEngine::setMidiEventLoggingEnabled(bool enable)
{
    if (audioEngine)
        audioEngine->setMidiEventLoggingEnabled(enable);
}

AudioEngine& AppEngine::getAudioEngine() { return *audioEngine; }
MIDIEngine& AppEngine::getMidiEngine() { return *midiEngine; }

int AppEngine::currentUndoTxn() const
{
    if (!edit)
        return 0;

    auto& um = edit->getUndoManager();

    return um.getUndoDescriptions().size();
}

bool AppEngine::isDirty() const noexcept
{
    return currentUndoTxn() != lastSavedTxn;
}

void AppEngine::markSaved()
{
    lastSavedTxn = currentUndoTxn();
}

bool AppEngine::writeEditToFile (const juce::File& file)
{
    if (! edit)
        return false;

    // 1) Flush plugin state into the edit's ValueTree
    for (auto* track : te::getAudioTracks (*edit))
    {
        if (! track)
            continue;

        for (auto* p : track->pluginList)
        {
            if (! p)
                continue;

            if (auto* morph = dynamic_cast<MorphSynthPlugin*> (p))
            {
                morph->saveToValueTree();
            }
            else if (auto* ext = dynamic_cast<te::ExternalPlugin*> (p))
            {
                ext->flushPluginStateToValueTree();
            }
        }
    }

    // 2) Now serialize the edit
    if (auto xml = edit->state.createXml())
    {
        juce::TemporaryFile tf (file);

        if (tf.getFile().replaceWithText (xml->toString())
            && tf.overwriteTargetFileWithTemporary())
        {
            DBG ("Saved edit to: " << file.getFullPathName());
            return true;
        }
    }

    return false;
}

bool AppEngine::saveEdit()
{
    if (!edit)
        return false;

    if (currentEditFile.getFullPathName().isNotEmpty())
    {
        const bool ok = writeEditToFile (currentEditFile);
        if (ok)
        {
            markSaved();
        }

        return ok;
    }

    saveEditAsAsync();
    return false;
}

void AppEngine::saveEditAsAsync (std::function<void (bool)> onDone)
{
    juce::File defaultDir = currentEditFile.existsAsFile()
                                ? currentEditFile.getParentDirectory()
                                : juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                      .getChildFile ("GrooveKit");
    defaultDir.createDirectory();

    auto chooser = std::make_shared<juce::FileChooser> ("Save Project As...",
        defaultDir,
        "*.tracktionedit;*.xml");

    chooser->launchAsync (juce::FileBrowserComponent::saveMode
                              | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser, onDone] (const juce::FileChooser& fc) {
            const auto result = fc.getResult();
            if (result == juce::File {})
            {
                if (onDone)
                    onDone (false);
                return;
            }

            auto chosen = result.withFileExtension (".tracktionedit");
            const bool ok = writeEditToFile (chosen);
            if (ok)
            {
                currentEditFile = chosen;
                if (edit)
                    edit->editFileRetriever = [f = currentEditFile] { return f; };
                markSaved();
            }
            if (onDone)
                onDone (ok);
        });
}

void AppEngine::setAutosaveMinutes (int minutes)
{
    if (minutes <= 0)
    {
        stopTimer();
        return;
    }
    startTimer (juce::jmax (1, minutes) * 60 * 1000);
}

juce::File AppEngine::getAutosaveFile() const
{
    if (currentEditFile.getFullPathName().isNotEmpty())
        return currentEditFile.getSiblingFile (currentEditFile.getFileNameWithoutExtension()
                                               + "_autosave.tracktionedit");
    return juce::File::getSpecialLocation (juce::File::tempDirectory)
        .getChildFile ("groovekit_autosave.tracktionedit");
}

void AppEngine::timerCallback()
{
    if (isDirty())
        writeEditToFile (getAutosaveFile());
}

void AppEngine::openEditAsync (std::function<void (bool)> onDone)
{
    auto startDir = currentEditFile.existsAsFile()
                        ? currentEditFile.getParentDirectory()
                        : juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                              .getChildFile ("GrooveKit");
    startDir.createDirectory();

    auto chooser = std::make_shared<juce::FileChooser> (
        "Open Project...", startDir, "*.tracktionedit;*.xml");

    chooser->launchAsync (juce::FileBrowserComponent::openMode
                              | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser, onDone] (const juce::FileChooser& fc) {
            auto f = fc.getResult();
            bool ok = false;
            if (f != juce::File {})
                ok = loadEditFromFile (f);

            if (onDone)
                onDone (ok);
        });
}

bool AppEngine::loadEditFromFile (const juce::File& file)
{
    closeInstrumentWindow();

    if (!file.existsAsFile() || !engine)
        return false;

    auto newEdit = tracktion::loadEditFromFile (*engine, file);
    if (!newEdit)
        return false;

    audioEngine.reset();

    edit = std::move (newEdit);
    currentEditFile = file;
    edit->editFileRetriever = [f = currentEditFile] { return f; };

    edit->getTransport().ensureContextAllocated();
    edit->clickTrackEmphasiseBars = true;  // Emphasize downbeats with different click sample

    markSaved();

    trackManager = std::make_unique<TrackManager> (*edit);
    midiEngine = std::make_unique<MIDIEngine> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);
    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    if (pluginManager)
        trackManager->setPluginManager (pluginManager.get());


    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    audioEngine->initialiseDefaults (48000.0, 512);
    audioEngine->setupMidiInputDevices(*edit);

    edit->restartPlayback();  // Rebuild playback graph with MIDI devices enabled

    for (auto* track : te::getAudioTracks (*edit))
    {
        if (!track) continue;

        for (auto* p : track->pluginList)
            if (auto* morph = dynamic_cast<MorphSynthPlugin*> (p))
                if (morph->state.isValid())
                    morph->restoreFromValueTree (morph->state);
    }

    markSaved();

    if (onEditLoaded)
        onEditLoaded();

    return true;
}

void AppEngine::openInstrumentEditor (int trackIndex)
{
    auto open = [this, trackIndex]
    {
        if (!trackManager) { DBG("AppEngine: no trackManager"); return; }
        if (auto* track = trackManager->getTrack (trackIndex))
        {
            te::Plugin* plug = nullptr;

            // your existing preference order…
            for (auto* p : track->pluginList)
                if (p && p->getPluginType() == MorphSynthPlugin::pluginType)
                { plug = p; break; }

            if (!plug)
                plug = trackManager->getInstrumentPluginOnTrack (trackIndex);

            if (!plug) { DBG("No instrument plugin on track " << trackIndex); return; }

            // MorphSynth path
            if (auto* morph = dynamic_cast<MorphSynthPlugin*>(plug))
            {
                if (instrumentWindow_) { instrumentWindow_->toFront (true); return; }
                instrumentWindow_ = std::make_unique<MorphSynthWindow>(
                    *morph,
                    [this]{ this->closeInstrumentWindow(); },
                    qwertyForwarder_.get()      // <— add this
                );
                instrumentWindow_->toFront (true);
                return;
            }

            if (auto* ext = dynamic_cast<te::ExternalPlugin*>(plug))
            {
                if (instrumentWindow_) { instrumentWindow_->toFront(true); return; }

                if (auto w = PluginEditorWindow::createFor(
                    *ext,
                    [this]{ this->closeInstrumentWindow(); },
                    qwertyForwarder_.get()   // <-- forward QWERTY to your MidiListener
                ))
                {
                    instrumentWindow_ = std::move(w);

                    // keep graph running so you can hear it while editing
                    auto& tc = edit->getTransport();
                    tc.ensureContextAllocated();
                    return;
                }


                DBG("ExternalPlugin has no instance/editor yet.");
                return;
            }


            auto& tc = edit->getTransport();
            tc.ensureContextAllocated();

            DBG("Plugin type has no editor route.");
        }
    };

    if (juce::MessageManager::getInstance()->isThisTheMessageThread()) open();
    else juce::MessageManager::callAsync (open);
}

void AppEngine::closeInstrumentWindow()
{
    if (instrumentWindow_ != nullptr)
    {
        instrumentWindow_->setVisible(false);
        instrumentWindow_.reset();
    }

    // If we started transport just to hear the editor, stop it now
    if (startedTransportForEditor_)
    {
        startedTransportForEditor_ = false;
        edit->getTransport().stop(/*discardRecordings=*/false, /*clearDevices=*/false);
    }
}

void AppEngine::showInstrumentChooser (int trackIndex)
{
    if (!trackManager || !pluginManager)
        return;

    juce::PopupMenu root, builtIn, external;

    // --- Built-in instruments
    const int kBuiltMorph = 1001;

    builtIn.addItem (kBuiltMorph, "Morph Synth");

    // --- External instruments (scanned)
    auto owned = pluginManager->getInstrumentDescriptions();   // OwnedArray<PluginDescription>
    juce::Array<juce::PluginDescription> descs;
    for (int i = 0; i < owned.size(); ++i)
        descs.add (*owned[i]);

    const int kExtBase = 2000;
    for (int i = 0; i < descs.size(); ++i)
    {
        const int id = kExtBase + i;
        external.addItem (id, descs[i].name + "  [" + descs[i].pluginFormatName + "]");
    }

    root.addSubMenu ("Built-in", std::move (builtIn));
    root.addSubMenu ("External", std::move (external));

    // Capture `descs` by value (copyable); no OwnedArray in the lambda
    root.showMenuAsync ({}, [this, trackIndex, descs] (int result)
    {
        if (result == 0 || !trackManager)
            return;

        te::Plugin* inserted = nullptr;

        if (result == 1001) // Morph
        {
            trackManager->clearInstrumentSlot0 (trackIndex);
            inserted = trackManager->insertMorphSynth (trackIndex);
        }
        else if (result >= 2000)
        {
            const int idx = result - 2000;
            if (idx >= 0 && idx < descs.size())
            {
                trackManager->clearInstrumentSlot0 (trackIndex);
                inserted = trackManager->insertExternalInstrument (trackIndex, descs.getReference (idx));
            }
        }

        if (inserted)
        {
            if (auto* track = trackManager->getTrack (trackIndex))
                wireAllMidiInputsToTrack (*track);

            // Notify UI that the instrument label should update
            if (onInstrumentLabelChanged)
                onInstrumentLabelChanged (trackIndex);

            openInstrumentEditor (trackIndex);
        }
    });

}

void AppEngine::onFxInsertSlotClicked (int trackIndex,
                                       int slotIndex,
                                       std::function<void (const juce::String&)> onSlotLabelChange)
{
    if (!trackManager)
        return;

    auto* track = trackManager->getTrack (trackIndex);
    if (!track)
        return;

    const int base = trackManager->getFxInsertBaseIndex (trackIndex);
    const int pluginIndex = base + slotIndex;

    te::Plugin* existing = nullptr;
    if (pluginIndex >= 0 && pluginIndex < track->pluginList.size())
        existing = track->pluginList[pluginIndex];

    if (auto* ext = dynamic_cast<te::ExternalPlugin*> (existing))
    {
        // 🔁 Same pattern as the ExternalPlugin branch in openInstrumentEditor
        if (instrumentWindow_)
        {
            instrumentWindow_->toFront (true);
            return;
        }

        if (auto w = PluginEditorWindow::createFor(
                *ext,
                [this] { this->closeInstrumentWindow(); },
                qwertyForwarder_.get()))
        {
            instrumentWindow_ = std::move (w);

            auto& tc = edit->getTransport();
            tc.ensureContextAllocated();
        }
        else
        {
            DBG ("[FX] ExternalPlugin has no instance/editor yet.");
        }

        return;
    }

    // No plugin in this slot yet -> show the FX menu and insert something
    showFxInsertMenu (trackIndex, slotIndex, std::move (onSlotLabelChange));
}

static void debugPrintPluginChain (te::Track* track)
{
    DBG ("===================== Plugin Chain Dump =====================");

    if (! track)
    {
        DBG ("(null track)");
        DBG ("==============================================================");
        return;
    }

    DBG ("Track: " << track->getName());

    int i = 0;
    for (auto* p : track->pluginList)
    {
        if (! p)
        {
            DBG ("  [" << i << "]  <null plugin>");
        }
        else
        {
            DBG ("  [" << i << "]  " << p->getPluginType() << " | " << p->getName());
        }
        ++i;
    }

    DBG ("==============================================================");
}

void AppEngine::showFxInsertMenu (int trackIndex,
                                  int slotIndex,
                                  std::function<void (const juce::String&)> onSlotLabelChange)
{
    if (! trackManager || ! pluginManager)
        return;

    // Get all scanned plugins and keep only non-instruments
    auto all = pluginManager->getAllPluginDescriptions();
    juce::Array<juce::PluginDescription> fxDescs;

    for (int i = 0; i < all.size(); ++i)
    {
        const auto& d = *all[i];

        // Skip instruments
        if (d.isInstrument)
            continue;

        if (isAppleBuiltInPlugin (d))
            continue;

        fxDescs.add (d);
    }


    if (fxDescs.isEmpty())
        return;

    // Root + category submenus (all external)
    juce::PopupMenu root, eqMenu, compMenu, reverbMenu, delayMenu, otherMenu;

    const int eqBase     = 2000;
    const int compBase   = 3000;
    const int reverbBase = 4000;
    const int delayBase  = 5000;
    const int otherBase  = 6000;

    for (int i = 0; i < fxDescs.size(); ++i)
    {
        const auto& d = fxDescs.getReference (i);
        auto displayName = d.name;

        auto lower = d.name.toLowerCase();

        if (lower.contains ("eq"))
            eqMenu.addItem (eqBase + i, displayName);
        else if (lower.contains ("comp"))
            compMenu.addItem (compBase + i, displayName);
        else if (lower.contains ("reverb") || lower.contains ("verb"))
            reverbMenu.addItem (reverbBase + i, displayName);
        else if (lower.contains ("delay") || lower.contains ("echo"))
            delayMenu.addItem (delayBase + i, displayName);
        else
            otherMenu.addItem (otherBase + i, displayName);
    }

    if (eqMenu.getNumItems()     > 0) root.addSubMenu ("EQ",         eqMenu);
    if (compMenu.getNumItems()   > 0) root.addSubMenu ("Compressor", compMenu);
    if (reverbMenu.getNumItems() > 0) root.addSubMenu ("Reverb",     reverbMenu);
    if (delayMenu.getNumItems()  > 0) root.addSubMenu ("Delay",      delayMenu);
    if (otherMenu.getNumItems()  > 0) root.addSubMenu ("Other FX",   otherMenu);

    if (root.getNumItems() == 0)
        return;

    const int kRemoveFxItem = 100;
    root.addItem (kRemoveFxItem, "Remove effect");

    root.showMenuAsync (juce::PopupMenu::Options(),
                    [this, trackIndex, slotIndex, fxDescs,
                     eqBase, compBase, reverbBase, delayBase, otherBase,
                     onSlotLabelChange] (int result)
    {
        if (result == 0 || ! trackManager)
            return;

        // --- Remove effect case ---
        if (result == kRemoveFxItem)
        {
            trackManager->clearFxInsertSlot (trackIndex, slotIndex);

            // Clear the label in the UI
            if (onSlotLabelChange)
                onSlotLabelChange ({});

            return;
        }
        int idx = -1;

        auto decode = [&] (int base)
        {
            if (result >= base && result < base + fxDescs.size())
                idx = result - base;
        };

        decode (eqBase);
        decode (compBase);
        decode (reverbBase);
        decode (delayBase);
        decode (otherBase);

        if (idx < 0 || idx >= fxDescs.size())
            return;

        const int base = trackManager->getFxInsertBaseIndex (trackIndex);
        const int insertIndex = base + slotIndex;

        trackManager->clearFxInsertSlot (trackIndex, slotIndex);

        // Insert the FX plugin via TrackManager / PluginManager
        te::Plugin* plugin = trackManager->insertExternalEffect (
            trackIndex,
            fxDescs.getReference (idx),
            insertIndex);

        auto currentTrack = trackManager->getTrack (trackIndex);
        debugPrintPluginChain (currentTrack);

        if (! plugin)
            return;

        // 1) Update the button label in the channel strip
        if (onSlotLabelChange)
            onSlotLabelChange (fxDescs.getReference (idx).name);

        // 2) Open the plugin editor right away (same pattern as instruments)
        if (auto* ext = dynamic_cast<te::ExternalPlugin*> (plugin))
        {
            if (instrumentWindow_)
            {
                instrumentWindow_->toFront (true);
                return;
            }

            if (auto w = PluginEditorWindow::createFor(
                    *ext,
                    [this] { this->closeInstrumentWindow(); },
                    qwertyForwarder_.get()))
            {
                instrumentWindow_ = std::move (w);

                auto& tc = edit->getTransport();
                tc.ensureContextAllocated();
            }
            else
            {
                DBG ("[FX] ExternalPlugin has no instance/editor yet (after insert).");
            }
        }
    });

}

juce::String AppEngine::getInstrumentLabelForTrack (int trackIndex) const
{
    if (!trackManager)
        return "Instrument";

    if (auto* plug = trackManager->getInstrumentPluginOnTrack (trackIndex))
    {
        // Prefer external plugin name if it is one, else generic plugin name
        if (auto* ext = dynamic_cast<te::ExternalPlugin*> (plug))
            return ext->getName();

        auto name = plug->getName();
        if (name.isNotEmpty())
            return name;
    }

    return "Instrument";
}

juce::String AppEngine::getInsertSlotLabel (int trackIndex, int slotIndex) const
{
    if (!trackManager)
        return {};

    auto* track = trackManager->getTrack (trackIndex);
    if (!track)
        return {};

    const int base = trackManager->getFxInsertBaseIndex (trackIndex);
    const int pluginIndex = base + slotIndex;

    if (pluginIndex < 0 || pluginIndex >= track->pluginList.size())
        return {};

    if (auto* plug = track->pluginList[pluginIndex])
    {
        if (auto* ext = dynamic_cast<te::ExternalPlugin*> (plug))
            return ext->getName();

        return plug->getName();
    }

    return {};
}

bool AppEngine::addMidiClipToTrackAt(int trackIndex, t::TimePosition start, t::BeatDuration length)
{
    if (!midiEngine)
        return false;

    return midiEngine->addMidiClipToTrackAt (trackIndex, start, length);
}

void AppEngine::copyMidiClip (te::MidiClip* clip)
{
    if (clip == nullptr || edit == nullptr)
        return;

    if (auto* cb = te::Clipboard::getInstance())
    {
        // Ensure the clip's runtime state is flushed to its ValueTree before copying
        clip->flushStateToValueTree();

        // Make a copy of the state and normalise timing so paste uses the insert point
        auto state = clip->state.createCopy();

        // Start should be relative to the paste insert point (0 seconds),
        // and we preserve only the clip's internal content offset.
        state.setProperty (te::IDs::start, 0.0, nullptr);
        state.setProperty (te::IDs::offset, clip->getPosition().getOffset().inSeconds(), nullptr);

        // Store whether this clip came from a drum track (Junie)
        bool isFromDrumTrack = false;
        if (auto* clipTrack = dynamic_cast<te::AudioTrack*> (clip->getTrack()))
        {
            const auto audioTracks = te::getAudioTracks (*edit);
            for (int i = 0; i < audioTracks.size(); ++i)
            {
                if (audioTracks[static_cast<size_t> (i)] == clipTrack)
                {
                    isFromDrumTrack = isDrumTrack (i);
                    break;
                }
            }
        }
        state.setProperty (GKIDs::isDrumClip, isFromDrumTrack, nullptr);

        // Store clip type info and length for paste validation (Written by Claude Code)
        lastCopiedClipWasDrum = isFromDrumTrack;
        lastCopiedClipLengthBeats = clip->getLengthInBeats().inBeats();
        hasClipboardTypeInfo = true;

        auto clips = std::make_unique<te::Clipboard::Clips>();
        // trackOffset = 0 keeps it on the same track relative to the insert point
        clips->addClip (0, state);

        cb->setContent (std::move (clips));
    }
}

bool AppEngine::pasteClipboardAt (const int trackIndex, const double startBeats)
{
    if (edit == nullptr)
        return false;

    const auto* cb = te::Clipboard::getInstance();
    if (cb == nullptr || cb->isEmpty())
        return false;

    const auto audioTracks = te::getAudioTracks (*edit);
    if (trackIndex < 0 || trackIndex >= audioTracks.size())
        return false;

    // Validate clip type matches target track type (Junie)
    if (hasClipboardTypeInfo)
    {
        const bool targetIsDrum = isDrumTrack (trackIndex);
        if (lastCopiedClipWasDrum != targetIsDrum)
        {
            // Clip type doesn't match track type - cannot paste
            DBG ("Cannot paste " << (lastCopiedClipWasDrum ? "drum" : "instrument")
                 << " clip to " << (targetIsDrum ? "drum" : "instrument") << " track");
            return false;
        }
    }

    te::EditInsertPoint ip (*edit);

    // Resolve target track and time from beats
    const auto targetTrack = te::Track::Ptr (audioTracks[static_cast<size_t> (trackIndex)]);
    const auto time = edit->tempoSequence.toTime (t::BeatPosition::fromBeats (startBeats));

    ip.setNextInsertPoint (time, targetTrack);

    if (const auto* content = cb->getContent())
    {
        // SelectionManager is optional; pass nullptr if you don't want selection behavior
        return content->pasteIntoEdit (*edit, ip, selectionManager.get());
    }

    return false;
}

bool AppEngine::duplicateMidiClip (te::MidiClip* clip)
{
    if (clip == nullptr || edit == nullptr)
        return false;

    // Compute destination start in beats – right after the source clip
    const double startBeats = clip->getStartBeat().inBeats();
    const double lenBeats   = clip->getLengthInBeats().inBeats();
    const double destBeats  = startBeats + lenBeats;

    // Determine the audio track index of the clip (must match getAudioTracks order)
    if (auto* clipTrack = dynamic_cast<te::AudioTrack*> (clip->getTrack()))
    {
        const auto audioTracks = te::getAudioTracks (*edit);
        int audioTrackIndex = -1;
        for (int i = 0; i < audioTracks.size(); ++i)
        {
            if (audioTracks[(size_t) i] == clipTrack)
            {
                audioTrackIndex = i;
                break;
            }
        }

        if (audioTrackIndex >= 0)
        {
            // Check for overlap before duplicating (Written by Claude Code)
            const auto destStartTime = edit->tempoSequence.toTime (t::BeatPosition::fromBeats (destBeats));
            const auto destEndTime = edit->tempoSequence.toTime (t::BeatPosition::fromBeats (destBeats + lenBeats));
            const t::TimeRange destRange (destStartTime, destEndTime);

            // Check if any clip on the track would overlap with the duplicate
            auto clipsOnTrack = getMidiClipsFromTrack (audioTrackIndex);
            for (auto* existingClip : clipsOnTrack)
            {
                if (existingClip == clip) continue; // Skip source clip

                auto existingRange = existingClip->getPosition().time;
                if (destRange.overlaps (existingRange))
                {
                    // Would overlap - don't duplicate
                    DBG ("Cannot duplicate clip - would overlap with existing clip at "
                         << existingRange.getStart().inSeconds() << "s");
                    return false;
                }
            }

            // No overlap - proceed with duplicate
            copyMidiClip (clip);
            return pasteClipboardAt (audioTrackIndex, destBeats);
        }
    }

    return false;
}

bool AppEngine::deleteMidiClip (te::MidiClip* clip)
{
    if (clip == nullptr || edit == nullptr)
        return false;

    // Remove the clip from its parent track. Tracktion will record this in the
    // edit state and make it undoable through the UndoManager.
    clip->removeFromParent();
    return true;
}

void AppEngine::importMidiClipViaChooser (int trackIndex,
                                          t::TimePosition destStart,
                                          std::function<void()> onSuccess)
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Import MIDI File",
        juce::File(),
        "*.mid;*.midi"
    );

    chooser->launchAsync (
        juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser, trackIndex, destStart, onSuccess] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (! file.existsAsFile())
            {
                return;
            }

            const bool ok = midiEngine->importMidiFileToTrack (file, trackIndex, destStart);

            if (! ok)
            {
                juce::AlertWindow::showMessageBoxAsync (
                    juce::AlertWindow::WarningIcon,
                    "MIDI Import Failed",
                    "Could not import the MIDI file into this track.");
            }
            else
            {
                if (onSuccess)
                    onSuccess();
            }
        }
    );
}

bool AppEngine::hasClipboardContent() const
{
    const auto* cb = te::Clipboard::getInstance();
    return cb != nullptr && !cb->isEmpty();
}

bool AppEngine::canPasteToTrack (int trackIndex) const
{
    // First check if clipboard has any content
    if (!hasClipboardContent())
        return false;

    // If we don't have type info, allow paste (backwards compatibility)
    if (!hasClipboardTypeInfo)
        return true;

    // Check if clip type matches track type
    const bool targetIsDrum = isDrumTrack (trackIndex);
    return lastCopiedClipWasDrum == targetIsDrum;
}

bool AppEngine::exportAudio (const juce::File& destFile)
{
    if (edit == nullptr)
        return false;

    auto parentDir = destFile.getParentDirectory();
    if (! parentDir.exists())
        parentDir.createDirectory();

    auto& transport = edit->getTransport();
    const bool wasPlaying = transport.isPlaying();
    if (wasPlaying)
        transport.stop (false, false);

    juce::BigInteger tracksToDo;
    {
        auto allTracks = te::getAllTracks (*edit);
        for (int i = 0; i < allTracks.size(); ++i)
            tracksToDo.setBit (i);
    }

    const auto start  = t::TimePosition::fromSeconds (0.0);
    const auto length = edit->getLength();
    const t::TimeRange range { start, length };

    const bool usePlugins = true;
    const bool useThread  = false;

    DBG ("[Export] Rendering audio to: " << destFile.getFullPathName());
    DBG ("[Export] Edit length (seconds): " << length.inSeconds());

    const bool ok = te::Renderer::renderToFile ("Export audio",
                                                destFile,
                                                *edit,
                                                range,
                                                tracksToDo,
                                                usePlugins,
                                                useThread);

    te::TransportControl::restartAllTransports (*engine, true);
    if (wasPlaying && ok)
        transport.play (false);

    DBG (juce::String ("[Export] Render ") + (ok ? "OK" : "FAILED"));

    return ok;
}
