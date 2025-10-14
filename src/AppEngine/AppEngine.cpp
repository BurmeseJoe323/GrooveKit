#include "AppEngine.h"
#include "../DrumSamplerEngine/DefaultSampleLibrary.h"
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

AppEngine::AppEngine()
{
    engine = std::make_unique<te::Engine> ("GrooveKitEngine");

    engine->getPluginManager().createPluginInstance = makeMessageThreadCreator();

    auto& pm = engine->getPluginManager();

    if (pm.pluginFormatManager.getNumFormats() == 0)
        pm.initialise();

    DBG("[PM] initialised once, formats=" << pm.pluginFormatManager.getNumFormats());
    for (int i = 0; i < pm.pluginFormatManager.getNumFormats(); ++i)
        DBG("  - " << pm.pluginFormatManager.getFormat(i)->getName());

    createOrLoadEdit();

    midiEngine = std::make_unique<MIDIEngine> (*edit);
    audioEngine = std::make_unique<AudioEngine> (*edit, *engine);
    trackManager = std::make_unique<TrackManager> (*edit);
    selectionManager = std::make_unique<te::SelectionManager> (*engine);

    editViewState = std::make_unique<EditViewState> (*edit, *selectionManager);

    audioEngine->initialiseDefaults (48000.0, 512);

#if JUCE_PLUGINHOST_VST3
    DBG("[HostFlags] VST3=1");
#else
    DBG("[HostFlags] VST3=0");
#endif
#if JUCE_PLUGINHOST_AU
    DBG("[HostFlags] AU=1");
#else
    DBG("[HostFlags] AU=0");
#endif
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

enum class ProbeResult { ok, failed, skipped };

static ProbeResult probeCreateJUCEInstance (tracktion::Engine& engine,
                                            const juce::PluginDescription& desc)
{
    // ✅ Use the engine's long-lived manager (like the demo does)
    auto& pfm = engine.getPluginManager().pluginFormatManager;

    juce::AudioPluginFormat* fmt = nullptr;
    for (int i = 0; i < pfm.getNumFormats(); ++i)
        if (pfm.getFormat(i)->getName() == desc.pluginFormatName)
            { fmt = pfm.getFormat(i); break; }

    if (fmt == nullptr)
    {
        DBG("[Probe] No format named: " << desc.pluginFormatName);
        return ProbeResult::failed;
    }

    double sr = engine.getDeviceManager().getSampleRate();
    int    bs = engine.getDeviceManager().getBlockSize();
    if (sr <= 0.0) sr = 48000.0;
    if (bs <= 0)   bs = 512;

    DBG("[Probe] begin for fmt=" << desc.pluginFormatName
                                 << " id=" << desc.fileOrIdentifier
                                 << " name=" << desc.name);

    const bool onMsgThread = juce::MessageManager::existsAndIsCurrentThread();
    if (onMsgThread)
    {
        // Keep callback alive until it fires (prevents dangling lambda capture)
        struct Holder : public juce::ReferenceCountedObject {
            juce::ReferenceCountedObjectPtr<Holder> keep { this };
        };
        auto* holder = new Holder();

        fmt->createPluginInstanceAsync(desc, sr, bs,
            [holder, d = desc](std::unique_ptr<juce::AudioPluginInstance> inst, const juce::String& err) mutable
            {
                if (inst) DBG("[Probe] (async) OK: " << inst->getName() << " [" << d.pluginFormatName << "]");
                else      DBG("[Probe] (async) FAILED: " << (err.isNotEmpty() ? err : "unknown"));
                holder->keep = nullptr; // release
            });

        DBG("[Probe] skipped waiting (on message thread)");
        return ProbeResult::skipped;
    }

    // Background thread → safe to wait
    juce::WaitableEvent done;
    std::unique_ptr<juce::AudioPluginInstance> instance;
    juce::String error;

    fmt->createPluginInstanceAsync(desc, sr, bs,
        [&](std::unique_ptr<juce::AudioPluginInstance> inst, const juce::String& err)
        {
            instance = std::move(inst); error = err; done.signal();
        });

    if (! done.wait (4000) || instance == nullptr)
    {
        DBG("[Probe] FAILED: " << (error.isNotEmpty() ? error : "timeout")
            << " | id=" << desc.fileOrIdentifier);
        return ProbeResult::failed;
    }

    DBG("[Probe] OK: " << instance->getName() << " [" << desc.pluginFormatName << "]");
    return ProbeResult::ok;
}


static juce::String findFirstVST3Instrument()
{
    juce::AudioPluginFormatManager fm;
    fm.addDefaultFormats();

    juce::VST3PluginFormat* vst3 = nullptr;
    for (int i = 0; i < fm.getNumFormats(); ++i)
        if (fm.getFormat(i)->getName() == "VST3")
        { vst3 = dynamic_cast<juce::VST3PluginFormat*>(fm.getFormat(i)); break; }

    if (vst3 == nullptr)
        return {};

    juce::KnownPluginList known;
    juce::FileSearchPath paths = vst3->getDefaultLocationsToSearch();
    juce::PluginDirectoryScanner scanner (known, *vst3, paths, false, juce::File());
    bool cancelled = false; juce::String name;
    while (scanner.scanNextFile (cancelled, name)) {}

    for (const auto& d : known.getTypes())
        if (d.isInstrument && d.pluginFormatName == "VST3")
            return d.fileOrIdentifier; // full .vst3 path

    return {};
}

juce::String AppEngine::getInstrumentNameForTrack (int trackIndex) const
{
    if (!edit) return "None";
    auto tracks = tracktion::getAudioTracks (*edit);
    if (trackIndex < 0 || trackIndex >= (int) tracks.size()) return "None";

    auto* t = tracks[(size_t) trackIndex];
    auto& list = t->pluginList;

    // Instrument is usually slot 0 and/or a synth
    for (int i = 0; i < list.size(); ++i)
        if (auto* p = list[i])
            if (p->isSynth() || i == 0) // conservative: treat slot 0 as the instrument
                return p->getName();

    return "None";
}

void AppEngine::debugPrintTrackPluginChain (int trackIndex)
{
    if (!edit) return;
    auto tracks = te::getAudioTracks (*edit);
    if (trackIndex < 0 || trackIndex >= (int) tracks.size()) return;
    auto* track = tracks[(size_t) trackIndex];

    DBG("[Chain] Track " << trackIndex << " \"" << track->getName()
        << "\" has " << track->pluginList.size() << " plugins:");
    for (int i = 0; i < track->pluginList.size(); ++i)
    {
        if (auto* p = track->pluginList[i])
        {
            if (auto* ext = dynamic_cast<te::ExternalPlugin*>(p))
            {
                auto* inst = ext->getAudioPluginInstance();
                DBG("  slot " << i << ": " << p->getName()
                    << " | [External] "
                    << (inst ? "instance=" + inst->getName() : "proc=null"));
            }
            else
            {
                DBG("  slot " << i << ": " << p->getName());
            }
        }
    }
}

// void AppEngine::printPluginDescriptions()
// {
//     juce::AudioPluginFormatManager fm;
//     fm.addDefaultFormats();
//
//     juce::KnownPluginList known;
//     juce::PluginDescription desc;
//     for (int i = 0; i < fm.getNumFormats(); ++i)
//     {
//         auto* format = fm.getFormat(i);
//         juce::StringArray searchPaths;
//         format->getDefaultLocations(searchPaths);
//         juce::PluginDirectoryScanner scanner(known, *format, searchPaths, true, juce::File());
//
//         bool cancelled = false;
//         while (scanner.scanNextFile(cancelled, desc))
//             DBG(desc.name + " | " + desc.pluginFormatName + " | Identifier: " + desc.fileOrIdentifier);
//     }
// }

std::optional<juce::PluginDescription>
AppEngine::resolveExactPluginDescription (const juce::String& idOrPath)
{
    juce::AudioPluginFormatManager fm;
    fm.addDefaultFormats(); // safe alongside Tracktion's engine

    for (int i = 0; i < fm.getNumFormats(); ++i)
    {
        auto* fmt = fm.getFormat(i);
        if (fmt == nullptr)
            continue;

        juce::KnownPluginList known;
        juce::PluginDescription desc;

        // 1) Get search paths for this format
        const juce::FileSearchPath paths = fmt->getDefaultLocationsToSearch();

        // 2) Create scanner (old JUCE signature)
        juce::PluginDirectoryScanner scanner (known, *fmt, paths,
                                              /*searchRecursively*/ false,
                                              /*deadMansPedalFile*/ juce::File());

        bool cancelled = false;
        juce::String nameBeingScanned;

        // 3) Scan plugins; scanner fills `known`
        while (scanner.scanNextFile (cancelled, nameBeingScanned))
        {
            // no-op; we just populate `known`
        }

        // 4) Match by identifier or path
        for (const auto& d : known.getTypes())
        {
            // Exact identifier match
            if (d.fileOrIdentifier == idOrPath)
                return d;

            // VST3: compare actual file paths
            if (d.pluginFormatName == "VST3")
            {
                juce::File want (idOrPath), have (d.fileOrIdentifier);
                if (want.existsAsFile() && have.existsAsFile()
                    && want.getFullPathName() == have.getFullPathName())
                    return d;
            }

            // AU: handle triplet and prefixed forms
            if (d.pluginFormatName == "AudioUnit")
            {
                auto auId = d.fileOrIdentifier;
                if (auId.containsChar (':'))
                    auId = auId.fromLastOccurrenceOf (":", false, false);

                if (idOrPath == auId
                    || idOrPath == ("AudioUnit:" + auId)
                    || idOrPath.containsIgnoreCase (auId))
                    return d;
            }
        }
    }

    return std::nullopt;
}

std::function<std::unique_ptr<juce::AudioPluginInstance>(const juce::PluginDescription&,
                                                         double, int, juce::String&)>
AppEngine::makeMessageThreadCreator()
{
    // Capture engine by pointer, not reference, to avoid lifetime gotchas.
    auto* eng = engine;
    return [eng](const juce::PluginDescription& desc,
                 double sr, int blockSize, juce::String& err) -> std::unique_ptr<juce::AudioPluginInstance>
    {
        if (eng == nullptr)
        {
            err = "Engine is null";
            return {};
        }

        // Find the requested format
        juce::AudioPluginFormat* fmt = nullptr;
        auto& pfm = eng->getPluginManager().pluginFormatManager;
        for (int i = 0; i < pfm.getNumFormats(); ++i)
            if (pfm.getFormat (i)->getName() == desc.pluginFormatName)
            { fmt = pfm.getFormat (i); break; }

        if (fmt == nullptr)
        {
            err = "Plugin format not available: " + desc.pluginFormatName;
            return {};
        }

        // Create on the message thread using the async factory
        std::unique_ptr<juce::AudioPluginInstance> instance;
        juce::WaitableEvent done;

        juce::MessageManager::callAsync ([fmt, desc, sr, blockSize, &instance, &err, &done]
        {
            fmt->createPluginInstanceAsync (desc, sr, blockSize,
                [&instance, &err, &done] (std::unique_ptr<juce::AudioPluginInstance> pi,
                                          const juce::String& error)
                {
                    err = error;
                    instance = std::move (pi);
                    done.signal();
                });
        });

        // Wait a little while for the instance to be created
        done.wait (8000); // 8s is generous; adjust if you like
        return instance; // may be nullptr if creation failed/timed out
    };
}

bool AppEngine::attachExternalInstrumentMinimal (int trackIndex, const juce::String& idOrPath)
{
    if (edit == nullptr)
        return false;

    auto tracks = te::getAudioTracks (*edit);
    if (trackIndex < 0 || trackIndex >= (int) tracks.size())
        return false;

    auto* track = tracks[(size_t) trackIndex];
    if (track == nullptr)
        return false;

    auto& tr = edit->getTransport();
    tr.ensureContextAllocated();

    // Remove any existing synth first
    for (int i = 0; i < track->pluginList.size(); ++i)
        if (auto* p = track->pluginList[i]; p && p->isSynth())
        {
            p->deleteFromParent();
            break;
        }

    // Use the resolver (note the leading :: if it's a free function)
    std::optional<juce::PluginDescription> resolved = resolveExactPluginDescription (idOrPath);

    juce::PluginDescription desc;
    if (resolved.has_value())
    {
        desc = *resolved;
        DBG("[Attach] Resolved plugin: " << desc.name
            << " | " << desc.pluginFormatName
            << " | id: " << desc.fileOrIdentifier);
    }
    else
    {
        const bool isVST3 = idOrPath.endsWithIgnoreCase (".vst3");
        desc.pluginFormatName  = isVST3 ? "VST3" : "AudioUnit";
        desc.fileOrIdentifier  = idOrPath;     // VST3: full bundle path; AU: exact triplet
        desc.isInstrument      = true;
        desc.name              = isVST3 ? juce::File(idOrPath).getFileNameWithoutExtension()
                                        : juce::String("External (AU)");
        desc.descriptiveName   = desc.name;
        desc.category          = "Instrument";
        desc.numInputChannels  = 0;
        desc.numOutputChannels = 2;

        DBG("[Attach] Fallback plugin desc: " << desc.name
            << " | " << desc.pluginFormatName
            << " | id: " << desc.fileOrIdentifier);
    }

    // ---- PROBE: verify JUCE can instantiate desc (any format) ----
    const auto probe = probeCreateJUCEInstance (*engine, desc);

    if (probe == ProbeResult::failed)
    {
        if (desc.pluginFormatName == "AudioUnit")
        {
            DBG("[Attach] AU probe failed. Falling back to a VST3 instrument if available…");

            if (auto fallbackVst3 = findFirstVST3Instrument(); fallbackVst3.isNotEmpty())
            {
                // overwrite desc to point at the VST3 instrument
                desc = {};
                desc.pluginFormatName  = "VST3";
                desc.fileOrIdentifier  = fallbackVst3;
                desc.isInstrument      = true;
                desc.name              = juce::File(fallbackVst3).getFileNameWithoutExtension();
                desc.descriptiveName   = desc.name;
                desc.category          = "Instrument";
                desc.numInputChannels  = 0;
                desc.numOutputChannels = 2;

                DBG("[Attach] Using VST3 fallback: " << desc.name
                    << " | id: " << desc.fileOrIdentifier);

                // Re-probe the chosen VST3 (don’t abort on 'skipped')
                const auto probeVst3 = probeCreateJUCEInstance (*engine, desc);
                if (probeVst3 == ProbeResult::failed)
                {
                    DBG("[Attach] VST3 probe also failed; aborting.");
                    return false;
                }
            }
            else
            {
                DBG("[Attach] No VST3 instrument found; aborting.");
                return false;
            }
        }
        else
        {
            DBG("[Attach] Probe failed for format " << desc.pluginFormatName << "; aborting.");
            return false;
        }
    }

    // If probe == ok or skipped, proceed:
    DBG("[Probe] passed or skipped; creating ExternalPlugin…");

    // Preferred path: ask PluginCache to create ExternalPlugin from type+desc
    te::Plugin::Ptr plugin = edit->getPluginCache()
        .createNewPlugin (te::ExternalPlugin::xmlTypeName, desc);

    if (plugin == nullptr)
    {
        DBG("[Attach] createNewPlugin(type, desc) failed; trying ValueTree path…");

        // Fallback: TE ExternalPlugin::create -> ValueTree -> materialise
        juce::ValueTree pluginState = te::ExternalPlugin::create (edit->engine, desc);
        if (! pluginState.isValid())
        {
            DBG("[Attach] ExternalPlugin::create returned invalid state for: "
                << desc.pluginFormatName << " / " << desc.fileOrIdentifier);
            return false;
        }

        plugin = edit->getPluginCache().createNewPlugin (pluginState);
        if (plugin == nullptr)
        {
            DBG("[Attach] PluginCache::createNewPlugin(ValueTree) failed for: "
                << desc.pluginFormatName << " / " << desc.fileOrIdentifier);
            return false;
        }
    }

    // Insert as instrument at slot 0
    track->pluginList.insertPlugin (plugin, 0, nullptr);
    plugin->setEnabled (true);
    plugin->initialiseFully();

    edit->restartPlayback();
    struct KickOnce : private juce::Timer
    {
        AppEngine* self {};
        te::Plugin::Ptr pluginKeepAlive {};
        int trackIndex {};

        KickOnce (AppEngine* s, te::Plugin::Ptr p, int ti)
            : self (s), pluginKeepAlive (std::move(p)), trackIndex (ti)
        {
            if (self == nullptr || self->edit == nullptr) { delete this; return; }

            auto& tr = self->edit->getTransport();
            tr.ensureContextAllocated();
            tr.setPosition (0.0_tp);
            tr.play (false);

            startTimer (900); // slightly longer window, ~0.45s
        }

        void timerCallback() override
        {
            stopTimer();

            if (self == nullptr || self->edit == nullptr) { delete this; return; }
            auto& tr = self->edit->getTransport();
            tr.stop (false, false);

            if (auto* ext = dynamic_cast<te::ExternalPlugin*>(pluginKeepAlive.get()))
            {
                auto* inst = ext->getAudioPluginInstance();
                DBG(juce::String("[ExtInst] after kick: ")
                    + (inst ? "instance OK (" + inst->getName() + ")" : "instance STILL null"));
            }
            self->debugPrintTrackPluginChain (trackIndex);
            delete this;
        }
    };
    new KickOnce (this, plugin, trackIndex);


    auto* self = this;
    te::Plugin::Ptr pluginKeepAlive = plugin;

    juce::MessageManager::callAsync([self, pluginKeepAlive, trackIndex]
    {
        if (self == nullptr || self->shuttingDown || self->edit == nullptr)
            return;

        if (auto* ext = dynamic_cast<te::ExternalPlugin*>(pluginKeepAlive.get()))
        {
            auto* inst = ext->getAudioPluginInstance();
            DBG(juce::String("[ExtInst] async check: ")
                + (inst ? "instance OK (" + inst->getName() + ")" : "instance STILL null"));
        }

        self->debugPrintTrackPluginChain (trackIndex);
    });


    return true;
}

