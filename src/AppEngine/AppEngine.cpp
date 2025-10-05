#include "AppEngine.h"
#include "../DrumSamplerEngine/DefaultSampleLibrary.h"
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

AppEngine::AppEngine()
{
    engine = std::make_unique<te::Engine> ("GrooveKitEngine");

    createOrLoadEdit();

    midiEngine = std::make_unique<MIDIEngine> (*edit);
    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    trackManager = std::make_unique<TrackManager> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);

    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    audioEngine->initialiseDefaults (48000.0, 512);
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
    edit = tracktion::createEmptyEdit (*engine, baseDir.getNonexistentChildFile ("Untitled", ".tracktionedit"));

    for (auto* t : tracktion::getAudioTracks (*edit))
        edit->deleteTrack (t);

    edit->editFileRetriever = [] { return juce::File {}; };
    edit->playInStopEnabled = true;

    markSaved();
    edit->restartPlayback();
}

void AppEngine::newUntitledEdit()
{
    audioEngine.reset();

    auto baseDir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                       .getChildFile ("GrooveKit");
    baseDir.createDirectory();

    auto placeholder = baseDir.getNonexistentChildFile ("Untitled", ".tracktionedit", false);

    edit = tracktion::createEmptyEdit (*engine, placeholder);

    currentEditFile = juce::File();

    edit->editFileRetriever = [placeholder] { return placeholder; };
    edit->playInStopEnabled = true;

    for (auto* t : tracktion::getAudioTracks (*edit))
        edit->deleteTrack (t);

    midiEngine = std::make_unique<MIDIEngine> (*edit);
    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    trackManager = std::make_unique<TrackManager> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);
    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    markSaved();

    audioEngine->initialiseDefaults (48000.0, 512);

    if (onEditLoaded)
        onEditLoaded();

    edit->restartPlayback();
}

void AppEngine::play() { audioEngine->play(); }

void AppEngine::stop() { audioEngine->stop(); }

bool AppEngine::isPlaying() const { return audioEngine->isPlaying(); }

void AppEngine::deleteMidiTrack (int index) { trackManager->deleteTrack (index); }

void AppEngine::addMidiClipToTrack (int trackIndex) { midiEngine->addMidiClipToTrack (trackIndex); }

te::MidiClip* AppEngine::getMidiClipFromTrack (int trackIndex)
{
    return midiEngine->getMidiClipFromTrack (trackIndex);
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
            markSaved();
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

    markSaved();

    if (onEditLoaded)
        onEditLoaded();

    return true;
}
