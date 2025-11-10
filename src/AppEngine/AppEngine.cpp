#include "AppEngine.h"
#include "../DrumSamplerEngine/DefaultSampleLibrary.h"
#include <tracktion_engine/tracktion_engine.h>
#include "../UI/Plugins/FourOsc/FourOscGUI.h"
#include "../UI/Plugins/Synthesizer/MorphSynthRegistration.h"
#include "../UI/Plugins/Synthesizer/MorphSynthView.h"
#include "../UI/Plugins/Synthesizer/MorphSynthWindow.h"

namespace te = tracktion::engine;
namespace t = tracktion;
using namespace std::literals;
using namespace t::literals;

AppEngine::AppEngine()
{
    engine = std::make_unique<te::Engine> ("GrooveKitEngine");
    registerMorphSynthCompat(*engine);

    createOrLoadEdit();

    midiEngine = std::make_unique<MIDIEngine> (*edit);
    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    trackManager = std::make_unique<TrackManager> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);
    midiListener = std::make_unique<MidiListener> (this);

    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    audioEngine->initialiseDefaults (48000.0, 512);

    // Setup MIDI input devices using Tracktion's InputDevice system
    audioEngine->setupMidiInputDevices(*edit);
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

    markSaved();
    edit->restartPlayback();
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

    for (auto* t : te::getAudioTracks (*edit))
        edit->deleteTrack (t);

    midiEngine = std::make_unique<MIDIEngine> (*edit);
    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    trackManager = std::make_unique<TrackManager> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);
    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    markSaved();

    audioEngine->initialiseDefaults (48000.0, 512);
    audioEngine->setupMidiInputDevices(*edit);

    if (onEditLoaded)
        onEditLoaded();

    edit->restartPlayback();
}

void AppEngine::setArmedTrack (int index)
{
    if (selectedTrackIndex == index)
        return;

    selectedTrackIndex = index;

    // Route MIDI to armed track (setTarget will update the routing automatically)
    if (index >= 0)
        audioEngine->routeMidiToTrack(*edit, index);

    if (onArmedTrackChanged)
        onArmedTrackChanged();
}

te::AudioTrack* AppEngine::getArmedTrack ()
{
    return getTrackManager().getTrack (selectedTrackIndex);
}

int AppEngine::getArmedTrackIndex () const
{
    return selectedTrackIndex;
}

void AppEngine::play() { audioEngine->play(); }

void AppEngine::stop() { audioEngine->stop(); }

bool AppEngine::isPlaying() const { return audioEngine->isPlaying(); }

void AppEngine::deleteMidiTrack (int index)
{
    if (index == selectedTrackIndex)
        selectedTrackIndex = -1;

    trackManager->deleteTrack (index);
}

bool AppEngine::addMidiClipToTrack (int trackIndex) { midiEngine->addMidiClipToTrack (trackIndex); }

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
    return trackManager->addInstrumentTrack();
}

void AppEngine::soloTrack (int i) { trackManager->soloTrack (i); }
void AppEngine::setTrackSoloed (int i, bool s) { trackManager->setTrackSoloed (i, s); }
bool AppEngine::isTrackSoloed (int i) const { return trackManager->isTrackSoloed (i); }
bool AppEngine::anyTrackSoloed() const { return trackManager->anyTrackSoloed(); }

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

AudioEngine& AppEngine::getAudioEngine() { return *audioEngine; }
MIDIEngine& AppEngine::getMidiEngine() { return *midiEngine; }

int AppEngine::currentUndoTxn() const
{
    if (!edit)
        return 0;
    if (auto xml = edit->state.createXml())
        return (int) xml->toString().hashCode();
    return 0;
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
    if (!edit)
        return false;

    for (auto* track : te::getAudioTracks (*edit))
    {
        if (! track) continue;

        for (auto* p : track->pluginList)
            if (auto* morph = dynamic_cast<MorphSynthPlugin*> (p))
                morph->saveToValueTree();  // <-- no assignment; it mutates plugin.state’s child
    }

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

    markSaved();

    trackManager = std::make_unique<TrackManager> (*edit);
    midiEngine = std::make_unique<MIDIEngine> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);
    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    audioEngine->initialiseDefaults (48000.0, 512);
    audioEngine->setupMidiInputDevices(*edit);

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
void AppEngine::makeFourOscAuditionPatch (int trackIndex)
{
    if (!trackManager) return;
    if (auto* plug = trackManager->getInstrumentPluginOnTrack (trackIndex))
    {
        auto params = plug->getAutomatableParameters();
        auto set = [&] (const juce::String& key, float norm)
        {
            for (auto* p : params)
                if (p && p->getParameterName().containsIgnoreCase (key))
                { p->setParameter (norm, juce::sendNotification); break; }
        };

        set ("Level 1",   1.00f);
        set ("Level 2",   0.00f);  set ("Level 3", 0.00f);  set ("Level 4", 0.00f);
        set ("Cutoff",    0.40f);
        set ("Resonance", 0.20f);
        set ("Amp Attack",  0.01f);
        set ("Amp Decay",   0.20f);
        set ("Amp Sustain", 0.80f);
        set ("Amp Release", 0.20f);
    }
}

void AppEngine::openInstrumentEditor (int trackIndex)
{
    auto open = [this, trackIndex]
    {
        if (!trackManager) { DBG("AppEngine: no trackManager"); return; }

        if (auto* track = trackManager->getTrack (trackIndex))
        {
            te::Plugin* plug = nullptr;

            // Prefer the first MorphSynth on the track
            for (auto* p : track->pluginList)
                if (p && p->getPluginType() == MorphSynthPlugin::pluginType)
                { plug = p; break; }

            // Fallback: whatever your old helper returns
            if (!plug)
                plug = trackManager->getInstrumentPluginOnTrack (trackIndex);

            if (plug)
            {
                if (auto* morph = dynamic_cast<MorphSynthPlugin*>(plug))
                {
                    // If already open, just bring it to front
                    if (instrumentWindow_ != nullptr)
                    {
                        instrumentWindow_->toFront (true);
                        return;
                    }

                    instrumentWindow_ = std::make_unique<MorphSynthWindow>(
                        *morph,
                        [this] { this->closeInstrumentWindow(); }
                    );

                    auto self = std::shared_ptr<AppEngine>(this, [] (AppEngine*) {});
                    static_cast<MorphSynthWindow*>(instrumentWindow_.get())->setAppEngine(self);

                    instrumentWindow_->toFront (true);
                    return;
                }

            }
            DBG("No instrument plugin found on track " << trackIndex);
        }
    };

    if (juce::MessageManager::getInstance()->isThisTheMessageThread()) open();
    else juce::MessageManager::callAsync (open);
}

void AppEngine::closeInstrumentWindow()
{
    if (instrumentWindow_ != nullptr)
    {
        instrumentWindow_->setVisible (false);
        instrumentWindow_.reset();
    }
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

    te::EditInsertPoint ip (*edit);

    // Resolve target track and time from beats
    const auto targetTrack = te::Track::Ptr (audioTracks[static_cast<size_t> (trackIndex)]);
    const auto time = edit->tempoSequence.toTime (t::BeatPosition::fromBeats (startBeats));

    ip.setNextInsertPoint (time, targetTrack);

    if (const auto* content = cb->getContent())
    {
        // SelectionManager is optional; pass nullptr if you don’t want selection behavior
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
