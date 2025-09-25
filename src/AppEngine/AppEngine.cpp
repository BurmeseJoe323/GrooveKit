
#include "AppEngine.h"
#include <tracktion_engine/tracktion_engine.h>
#include "../DrumSamplerEngine/DefaultSampleLibrary.h"

namespace te = tracktion;
using namespace juce;
using namespace std::literals;
using namespace te::literals;
namespace {
    static constexpr const char* kFourOSCId    = "FourOsc";
    static constexpr const char* kDrumSamplerId= "DrumSampler";
}

namespace {
    static juce::File pluginCacheFile()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                     .getChildFile ("GrooveKit");
        dir.createDirectory();
        return dir.getChildFile ("known_plugins.xml");
    }
}

AppEngine::AppEngine()
{
    engine = std::make_unique<te::Engine>("GrooveKitEngine");

    createOrLoadEdit();

    midiEngine = std::make_unique<MIDIEngine>(*edit);
    audioEngine = std::make_unique<AudioEngine>(*edit, *engine);
    trackManager = std::make_unique<TrackManager>(*edit); 
    selectionManager = std::make_unique<te::SelectionManager>(*engine);

    trackManager->setSelectionManager(selectionManager.get());

    editViewState = std::make_unique<EditViewState>(*edit, *selectionManager);

    audioEngine->initialiseDefaults (48000.0, 512);

    pluginManager = std::make_unique<PluginManager>();

    const auto cache = pluginCacheFile();
    pluginManager->rescanAllAndSave (cache, /*clearBlacklist*/ true);

    DBG ("[Plugins] Known now: " << pluginManager->getKnownList().getNumTypes());
    if (cache.existsAsFile())
        pluginManager->load (cache);

    if (pluginManager->getKnownList().getNumTypes() == 0)
    {
        DBG ("[Plugins] Scanning default paths…");
        pluginManager->scanDefaultPaths();
        pluginManager->save (cache);
        DBG ("[Plugins] Found: " << pluginManager->getKnownList().getNumTypes());
    }
    else
    {
        DBG ("[Plugins] Loaded cached list: " << pluginManager->getKnownList().getNumTypes());
    }

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

AppEngine::~AppEngine() = default;


void AppEngine::createOrLoadEdit()
{
    auto baseDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                       .getChildFile("GrooveKit");
    baseDir.createDirectory();

    currentEditFile = "";
    edit = te::createEmptyEdit (*engine, baseDir.getNonexistentChildFile("Untitled", ".tracktionedit"));

    for (auto* t : te::getAudioTracks(*edit))
        edit->deleteTrack(t);

    edit->editFileRetriever = [] { return juce::File{}; };
    edit->playInStopEnabled = true;

    markSaved();
    edit->restartPlayback();

}

void AppEngine::newUntitledEdit()
{
    audioEngine.reset();

    auto baseDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                       .getChildFile("GrooveKit");
    baseDir.createDirectory();

    auto placeholder = baseDir.getNonexistentChildFile("Untitled", ".tracktionedit", false);

    edit = te::createEmptyEdit(*engine, placeholder);

    currentEditFile = juce::File();

    edit->editFileRetriever = [placeholder]{ return placeholder; };
    edit->playInStopEnabled = true;

    for (auto* t : te::getAudioTracks(*edit))
        edit->deleteTrack(t);

    midiEngine        = std::make_unique<MIDIEngine>(*edit);
    audioEngine       = std::make_unique<AudioEngine>(*edit, *engine);
    trackManager      = std::make_unique<TrackManager>(*edit);
    selectionManager  = std::make_unique<te::SelectionManager>(*engine);
    editViewState     = std::make_unique<EditViewState>(*edit, *selectionManager);

    markSaved();

    audioEngine->initialiseDefaults (48000.0, 512);

    if (onEditLoaded) onEditLoaded();

    edit->restartPlayback();
}

void AppEngine::play() { audioEngine->play(); }

void AppEngine::stop() { audioEngine->stop(); }

void AppEngine::deleteMidiTrack(int index) { trackManager->deleteTrack(index); }

void AppEngine::addMidiClipToTrack(int trackIndex) { midiEngine->addMidiClipToTrack(trackIndex); }

te::MidiClip *AppEngine::getMidiClipFromTrack(int trackIndex) {
    return midiEngine->getMidiClipFromTrack(trackIndex);
}

int AppEngine::getNumTracks() const
{
    auto v = te::getAudioTracks (*edit);
    return (int) v.size();
}

EditViewState &AppEngine::getEditViewState() { return *editViewState; }

te::Edit & AppEngine::getEdit() { return *edit; }

bool AppEngine::isDrumTrack (int i) const{ return trackManager ? trackManager->isDrumTrack(i) : false;}

DrumSamplerEngineAdapter* AppEngine::getDrumAdapter (int i){ return trackManager ? trackManager->getDrumAdapter(i) : nullptr;}

int AppEngine::addMidiTrack ()
{
    if (!trackManager) return -1;

    if (auto* t = trackManager->addMidiTrack ("MIDI Track"))
    {
        // Find actual index of the new track
        auto tracks = te::getAudioTracks (*edit);
        for (int i = 0; i < (int) tracks.size(); ++i)
            if (tracks[(size_t) i] == t)
                return i;
    }
    return -1;
}



int AppEngine::addDrumTrack(){
    jassert (trackManager != nullptr);
    return trackManager->addDrumTrack();
}

int AppEngine::addInstrumentTrack(){
    jassert (trackManager != nullptr);
    return trackManager->addInstrumentTrack();
}

void AppEngine::soloTrack(int i)                 { trackManager->soloTrack(i); }
void AppEngine::setTrackSoloed(int i, bool s)    { trackManager->setTrackSoloed(i, s); }
bool AppEngine::isTrackSoloed(int i) const       { return trackManager->isTrackSoloed(i); }
bool AppEngine::anyTrackSoloed() const           { return trackManager->anyTrackSoloed(); }

AudioEngine& AppEngine::getAudioEngine() { return *audioEngine; }
MIDIEngine&  AppEngine::getMidiEngine()  { return *midiEngine;  }

int AppEngine::currentUndoTxn() const
{
    if (!edit) return 0;
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
    if (!edit) return false;

    if (auto xml = edit->state.createXml())
    {
        juce::TemporaryFile tf (file);
        if (tf.getFile().replaceWithText (xml->toString())
            && tf.overwriteTargetFileWithTemporary())
        {
            DBG("Saved edit to: " << file.getFullPathName());
            return true;
        }
    }
    return false;
}

bool AppEngine::saveEdit()
{
    if (!edit) return false;

    if (currentEditFile.getFullPathName().isNotEmpty())
    {
        const bool ok = writeEditToFile (currentEditFile);
        if (ok) markSaved();
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
        [this, chooser, onDone] (const juce::FileChooser& fc)
        {
            const auto result = fc.getResult();
            if (result == juce::File{})
            {
                if (onDone) onDone(false);
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
            if (onDone) onDone(ok);
        });
}

void AppEngine::setAutosaveMinutes (int minutes)
{
    if (minutes <= 0) { stopTimer(); return; }
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

    auto chooser = std::make_shared<juce::FileChooser>(
        "Open Project...", startDir, "*.tracktionedit;*.xml");

    chooser->launchAsync (juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser, onDone] (const juce::FileChooser& fc)
        {
            auto f = fc.getResult();
            bool ok = false;
            if (f != juce::File{})
                ok = loadEditFromFile (f);

            if (onDone) onDone (ok);
        });
}

bool AppEngine::loadEditFromFile (const juce::File& file)
{
    if (! file.existsAsFile() || ! engine) return false;

    auto newEdit = te::loadEditFromFile(*engine, file);
    if (! newEdit) return false;

    audioEngine.reset();

    edit = std::move(newEdit);
    currentEditFile = file;
    edit->editFileRetriever = [f = currentEditFile] { return f; };

    edit->getTransport().ensureContextAllocated();

    markSaved();

    trackManager     = std::make_unique<TrackManager>(*edit);
    midiEngine       = std::make_unique<MIDIEngine>(*edit);
    selectionManager = std::make_unique<te::SelectionManager>(*engine);
    editViewState    = std::make_unique<EditViewState>(*edit, *selectionManager);

    audioEngine      = std::make_unique<AudioEngine>(*edit, *engine);
    audioEngine->initialiseDefaults(48000.0, 512);

    markSaved();

    if (onEditLoaded) onEditLoaded();

    return true;
}

std::vector<AppEngine::InstrumentChoice> AppEngine::getAvailableInstruments() const
{
    std::vector<InstrumentChoice> out;

    // Built-ins first
    out.push_back({ InstrumentChoice::Kind::FourOSC,     "FourOSC",      std::nullopt });
    out.push_back({ InstrumentChoice::Kind::DrumSampler, "Drum Sampler", std::nullopt });

    // External instruments
    auto& kpl = pluginManager->getKnownList();
    for (const auto& d : kpl.getTypes())        // d is a PluginDescription&
        if (d.isInstrument)
            out.push_back({ InstrumentChoice::Kind::External, d.name, d }); // copy d into optional

    return out;
}


static int findInstrumentIndex (te::PluginList& list)
{
    for (int i = 0; i < list.size(); ++i)
        if (auto* p = list[i])
            if (p->isSynth())
                return i;

    return -1;
}

static void removeExistingInstrument (te::PluginList& list)
{
    const int idx = findInstrumentIndex (list);
    if (idx >= 0)
        if (auto* p = list[idx])
            p->deleteFromParent();       // removes from its PluginList (with undo)
}

static juce::ValueTree makePluginStateForType (const char* xmlTypeName)
{
    // Root must be a PLUGIN node with a "type" property
    juce::ValueTree v (te::IDs::PLUGIN);
    v.setProperty (te::IDs::type, juce::String (xmlTypeName), nullptr);
    return v;
}

bool AppEngine::setTrackInstrument (te::AudioTrack& track, const InstrumentChoice& choice)
{
    te::Plugin::Ptr plugin;

    if (choice.kind == InstrumentChoice::Kind::External && choice.external.has_value())
    {
        plugin = edit->getPluginCache()
                     .createNewPlugin (te::ExternalPlugin::xmlTypeName, *choice.external);
        if (plugin == nullptr)
        {
            DBG ("[Instrument] External create failed for "
                 << choice.external->name << " (" << choice.external->pluginFormatName << ")");
            return false;
        }
        descByPlugin[plugin.get()] = *choice.external;
    }
    else if (choice.kind == InstrumentChoice::Kind::FourOSC)
    {
        auto state  = makePluginStateForType (te::FourOscPlugin::xmlTypeName);
        plugin = edit->getPluginCache().createNewPlugin (state);
        if (plugin == nullptr)
        {
            DBG("[Instrument] FourOSC create failed");
            return false;
        }
    }
    else if (choice.kind == InstrumentChoice::Kind::DrumSampler)
    {
        // We *don’t* insert a plugin. This track is a Drum track driven by your adapter.
        removeExistingInstrument (track.pluginList);

        // Mark as drum and (re)build the adapter mapping
        track.state.setProperty (juce::Identifier ("gk_isDrum"), true, nullptr);
        if (trackManager) trackManager->syncBookkeepingToEngine();

        // Make sure the graph exists
        if (edit) edit->getTransport().ensureContextAllocated();
        return true; // early out – no plugin to insert
    }


    if (plugin == nullptr) return false;

    track.pluginList.insertPlugin (plugin, 0, nullptr);
    plugin->setEnabled (true);

    if (edit) edit->getTransport().ensureContextAllocated();
    plugin->initialiseFully();

    DBG("[INST] chain after setTrackInstrument for " << track.getName());
    for (int i = 0; i < track.pluginList.size(); ++i)
        if (auto* p2 = track.pluginList[i])
            DBG("  slot " << i << ": " << p2->getName());

    if (trackManager) trackManager->syncBookkeepingToEngine();

    // (optional debug)
    DBG("[INST] chain after setTrackInstrument for " << track.getName());
    for (int i = 0; i < track.pluginList.size(); ++i)
        if (auto* p2 = track.pluginList[i])
            DBG("  slot " << i << ": " << p2->getName());

    return true;
}


bool AppEngine::addEffectToTrack (te::AudioTrack& track,
                                  const juce::PluginDescription& desc)
{
    if (desc.isInstrument)      // safety: FX only
        return false;

    auto& list = track.pluginList;

    // Create an ExternalPlugin instance from the PluginDescription
    te::Plugin::Ptr plugin =
        edit->getPluginCache()
            .createNewPlugin (te::ExternalPlugin::xmlTypeName, desc);
    const auto cache = pluginCacheFile();
    if (cache.existsAsFile())
    {
        pluginManager->load (cache);
        DBG ("[Plugins] Loaded cached list: " << pluginManager->getKnownList().getNumTypes());
    }
    else
    {
        pluginManager->rescanAllAndSave (cache);
    }

    if (plugin == nullptr)
    {
        DBG ("[FX] Failed to create: " << desc.name);
        return false;
    }

    // Insert at end of the chain (after instrument/other FX)
    const int insertIndex = list.size();

    // 3rd arg is SelectionManager* in most TE versions; pass yours or nullptr
    list.insertPlugin (plugin, insertIndex, /* selectionManager */ nullptr);

    plugin->setEnabled (true);
    return true;
}

namespace {
    struct EditorWhenReady : juce::Timer
    {
        AppEngine* eng = nullptr;
        te::AudioTrack* track = nullptr;
        juce::WeakReference<juce::Component> anchor;
        int attempts = 0;

        EditorWhenReady (AppEngine* e, te::AudioTrack* t, juce::Component* a)
            : eng(e), track(t), anchor(a) { startTimer (80); }

        void timerCallback() override
        {
            if (++attempts > 40 || eng == nullptr || track == nullptr)
            {
                DBG("[INST] editor wait timeout");
                stopTimer(); delete this; return;
            }

            if (auto* p = eng->findInstrument (*track))
                if (auto* ext = dynamic_cast<te::ExternalPlugin*> (p))
                    if (ext->getAudioPluginInstance() != nullptr)
                    {
                        DBG("[INST] editor ready after " << attempts << " ticks");
                        stopTimer();
                        if (auto* p = eng->findInstrument (*track))
                            if (auto* ext = dynamic_cast<te::ExternalPlugin*> (p))
                                if (auto* proc = ext->getAudioPluginInstance())
                                {
                                    DBG("[AUDIT] (ready) outs=" << proc->getTotalNumOutputChannels()
                                        << " acceptsMidi=" << int(proc->acceptsMidi()));
                                    eng->openPluginWindow (*p, anchor.get());
                                    stopTimer(); delete this; return;
                                }
                        eng->openPluginWindow (*p, anchor.get());
                        delete this;
                    }
        }
    };
}

void AppEngine::openInstrumentWhenReady (te::AudioTrack& track, juce::Component* anchor)
{
    if (edit)
    {
        auto& tr = edit->getTransport();
        tr.ensureContextAllocated();
        // Log whether a context actually exists:
        DBG("[CTX] " << (tr.getCurrentPlaybackContext() ? "OK" : "NULL"));
    }
    new EditorWhenReady (this, &track, anchor); // self-deletes
}


void AppEngine::showInstrumentMenuForTrack(te::AudioTrack& track,
                                           juce::Component& anchor,
                                           std::function<void()> onChanged)
{
    auto choices = getAvailableInstruments();

    juce::PopupMenu builtIn, external, root;
    int id = 1; std::map<int, InstrumentChoice> map;

    for (auto& c : choices)
    {
        (c.kind == InstrumentChoice::Kind::External ? external : builtIn)
            .addItem(id, c.displayName);
        map[id++] = c;
    }

    // Optional: "None" item to remove instrument
    builtIn.addSeparator();
    const int noneId = id;
    builtIn.addItem(noneId, "None (Remove Instrument)");
    ++id;

    if (builtIn.getNumItems() > 0)  root.addSubMenu("Built-in", builtIn);
    if (external.getNumItems() > 0) root.addSubMenu("External", external);

    root.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&anchor),
        [this,
                    &track,
                    anchorPtr = &anchor,
                    map = std::move(map),
                    noneId,
                    onChanged = std::move(onChanged)](int result)
        {
            bool changed = false;



            if (result == noneId)
            {
                removeExistingInstrument(track.pluginList);
                changed = true;
            }
            else if (auto it = map.find(result); it != map.end())
            {
                if (it->second.kind == InstrumentChoice::Kind::External && it->second.external)
                {
                    const auto& d = *it->second.external;
                    DBG("[Picker] " << d.name << " fmt=" << d.pluginFormatName
                        << " id=" << d.fileOrIdentifier);
                }
                changed = setTrackInstrument(track, it->second);
                if (changed)
                {
                    // Ensure Tracktion allocates a playback context now
                    if (edit)
                    {
                        auto& tr = edit->getTransport();
                        tr.ensureContextAllocated();

                        // tiny nudge to force graph/processor construction
                        tr.setPosition (0_tp);
                        tr.play (false);
                        juce::Timer::callAfterDelay (60, [&]{ tr.stop (false, false); });
                    }
                    // Audit: did Tracktion build a processor for the chosen instrument?
                    if (auto* p = findInstrument (track))
                        if (auto* ext = dynamic_cast<te::ExternalPlugin*> (p))
                            if (auto* proc = ext->getAudioPluginInstance())
                                DBG("[AUDIT] proc outs=" << proc->getTotalNumOutputChannels()
                                    << " acceptsMidi=" << int(proc->acceptsMidi()));

                    // Schedule opening the REAL instance’s editor as soon as it exists
                    openInstrumentWhenReady (track, anchorPtr);
                }

            }

            if (changed && onChanged)
                onChanged();
        });
}

void AppEngine::showFxMenuForTrack (te::AudioTrack& track, juce::Component& anchor)
{
    juce::PopupMenu fx; int id = 1;
    std::map<int, juce::PluginDescription> map;

    auto& kpl = pluginManager->getKnownList();
    for (const auto& d : kpl.getTypes())
        if (!d.isInstrument) { fx.addItem (id, d.name); map[id++] = d; }

    fx.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&anchor),
        [this, &track, map = std::move(map)] (int result)
        {
            if (result > 0)
                if (auto it = map.find (result); it != map.end())
                    addEffectToTrack (track, it->second);
        });
}

struct ExternalEditorWindow : public juce::DocumentWindow
{
    ExternalEditorWindow (juce::Component* editorComp, const juce::String& title)
        : juce::DocumentWindow (title, juce::Colours::black, DocumentWindow::closeButton)
    {
        setUsingNativeTitleBar (true);
        setContentOwned (editorComp, true);
        centreWithSize (editorComp->getWidth(), editorComp->getHeight());
        setAlwaysOnTop (true);
        setVisible (true);
    }

    void closeButtonPressed() override
    {
        setVisible (false);
        delete this; // self-owning
    }
};

// Self-owning window to host any JUCE Component (editor or generic)
struct PluginEditorWindow : public juce::DocumentWindow,
                            private juce::ComponentListener
{
    PluginEditorWindow (juce::Component* c,
                        const juce::String& title,
                        juce::Component* anchorTo = nullptr)
        : juce::DocumentWindow (title, juce::Colours::black, closeButton)
    {
        setUsingNativeTitleBar (true);
        setResizable (true, false);

        // Take ownership of the editor
        setContentOwned (c, true);
        c->addComponentListener (this);

        // Some plugins report 0x0 initially → give a sensible default
        int w = c->getWidth(), h = c->getHeight();
        if (w <= 0 || h <= 0) { w = 640; h = 420; }

        if (anchorTo != nullptr)
            centreAroundComponent (anchorTo, w, h);
        else
            centreWithSize (w, h);

        setVisible (true);
    }

    void closeButtonPressed() override { delete this; }

    // If the editor later resizes, keep the window snug
    void componentResized (juce::Component* c)
    {
        if (c == getContentComponent())
            setSize (juce::jmax (c->getWidth(), 320),
                     juce::jmax (c->getHeight(), 200));
    }
};

namespace {
    struct TempPluginWindow : public juce::DocumentWindow
    {
        std::unique_ptr<juce::AudioPluginInstance> inst;

        TempPluginWindow (std::unique_ptr<juce::AudioPluginInstance> p,
                          double sr, int bs, juce::Component* anchor)
            : juce::DocumentWindow ("Plugin (temp)", juce::Colours::black, closeButton),
              inst (std::move (p))
        {
            setUsingNativeTitleBar (true);
            setResizable (true, false);

            // Make sure the processor is ready -> avoids JUCE assertions
            inst->prepareToPlay (sr, bs);

            if (auto* ed = inst->createEditorIfNeeded())
            {
                if (ed->getWidth() <= 0 || ed->getHeight() <= 0) ed->setSize (640, 420);
                setContentOwned (ed, true);
            }
            else
            {
                // fallback generic editor
                setContentOwned (new juce::GenericAudioProcessorEditor (*inst), true);
            }

            if (anchor != nullptr) centreAroundComponent (anchor, getWidth(), getHeight());
            else                    centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            if (inst) { inst->releaseResources(); }
            delete this;
        }
    };
}

void AppEngine::openPluginWindow (te::Plugin& p, juce::Component* anchor)
{
    auto run = [this, pluginPtr = &p, anchor]
    {
        if (edit) edit->getTransport().ensureContextAllocated();

        DBG("[INST] Opening: " << pluginPtr->getName());

        // External plugins (VST3/AU) – use the REAL graph instance only
        if (auto* ext = dynamic_cast<te::ExternalPlugin*> (pluginPtr))
        {
            if (auto* proc = ext->getAudioPluginInstance())
            {
                auto showEditor = [anchor](juce::Component* c, const juce::String& title)
                {
                    juce::DialogWindow::LaunchOptions opt;
                    opt.dialogTitle = title;
                    opt.content.setOwned (c);
                    opt.escapeKeyTriggersCloseButton = true;
                    opt.useNativeTitleBar = true;
                    opt.resizable = true;
                    if (c->getWidth() <= 0 || c->getHeight() <= 0) c->setSize (640, 420);
                    opt.componentToCentreAround = anchor;
                    opt.launchAsync();
                };

                if (auto* ed = proc->createEditorIfNeeded())
                {
                    DBG("[INST] vendor editor OK, size="
                        << ed->getWidth() << "x" << ed->getHeight());
                    showEditor (ed, proc->getName());
                    return;
                }

                DBG("[INST] vendor editor NULL, using GenericAudioProcessorEditor");
                showEditor (new juce::GenericAudioProcessorEditor (*proc), proc->getName());
                return;
            }

            // Processor not ready yet – schedule once and return
            DBG("[INST] processor not ready yet; schedule open later");
            if (auto* owner = pluginPtr->getOwnerTrack())
                if (auto* audio = dynamic_cast<te::AudioTrack*> (owner))
                    openInstrumentWhenReady (*audio, anchor);
            return;
        }

        // Built-ins (Drum Sampler, etc.) – your existing path if any
        // if (engine) { engine->getUIBehaviour().showPluginWindow (pluginPtr); return; }

        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::InfoIcon,
                                                "Instrument",
                                                "No editor available for this instrument.");
    };

    if (!juce::MessageManager::getInstance()->isThisTheMessageThread())
        juce::MessageManager::callAsync (run);
    else
        run();
}


static juce::String getInstrumentName (const te::AudioTrack& track)
{
    auto& list = track.pluginList;
    for (int i = 0; i < list.size(); ++i)
        if (auto* p = list[i]; p && p->isSynth())
            return p->getName();
    return "None";
}

void AppEngine::removeFxAt (te::AudioTrack& track, int idx)
{
    if (auto* p = track.pluginList[idx]) p->deleteFromParent();
}

void AppEngine::setFxBypassed (te::AudioTrack& track, int idx, bool bypass)
{
    if (auto* p = track.pluginList[idx])
    {
        // External plugins: use JUCE's bypass parameter if available
        if (auto* ext = dynamic_cast<te::ExternalPlugin*>(p))
            if (auto* inst = ext->getAudioPluginInstance())
                if (auto* bp = inst->getBypassParameter())
                {
                    bp->beginChangeGesture();
                    bp->setValueNotifyingHost(bypass ? 1.0f : 0.0f);  // 1 = bypassed
                    bp->endChangeGesture();
                    return;
                }

        // Fallback for plugins without a bypass param (incl. some built-ins)
        p->setEnabled(!bypass);
    }
}



bool AppEngine::isFxBypassed (te::AudioTrack& track, int idx) const
{
    if (auto* p = track.pluginList[idx])
    {
        if (auto* ext = dynamic_cast<te::ExternalPlugin*>(p))
            if (auto* inst = ext->getAudioPluginInstance())
                if (auto* bp = inst->getBypassParameter())
                    return bp->getValue() >= 0.5f; // 1 = bypassed

        return !p->isEnabled(); // treat disabled as bypassed
    }
    return false;
}

static te::Plugin* findInstrumentImpl (te::AudioTrack& track)
{
    auto& list = track.pluginList;
    for (int i = 0; i < list.size(); ++i)
        if (auto* p = list[i]; p && p->isSynth())
            return p;
    return nullptr;
}


static bool isInstrumentPlugin (te::Plugin& p)
{
    // 1) Tracktion reports some as synths
    if (p.isSynth())
        return true;

    // 2) External VST3/AU: instrument if it accepts MIDI and produces audio (not a MIDI effect)
    if (auto* ext = dynamic_cast<te::ExternalPlugin*> (&p))
        if (auto* inst = ext->getAudioPluginInstance())
            return (inst->acceptsMidi() || inst->getNumInputChannels() == 0)
                   && inst->getTotalNumOutputChannels() > 0
                   && !inst->isMidiEffect();

    // 3) Built-ins / custom: whitelist known instrument classes
    // Uncomment the ones you use:
    // if (dynamic_cast<te::FourOscPlugin*> (&p))      return true;
    // if (dynamic_cast<te::SamplerPlugin*> (&p))      return true;      // Tracktion Sampler
    // if (dynamic_cast<YourDrumSamplerPlugin*> (&p))  return true;      // your own sampler class

    return false;
}

te::Plugin* AppEngine::findInstrument (te::AudioTrack& track)
{
    if (edit) edit->getTransport().ensureContextAllocated();

    auto& list = track.pluginList;

    for (int i = 0; i < list.size(); ++i)
        if (auto* p = list[i])
        {
            // Built-ins that report synth status
            if (p->isSynth())
                return p;

            // Your custom Sampler by name (or switch to a class check in your build)
            if (p->getName().equalsIgnoreCase ("Sampler"))
                return p;

            // External VST3/AU:
            if (auto* ext = dynamic_cast<te::ExternalPlugin*> (p))
            {
                // If it's the first plugin in the chain, we treat it as "the instrument"
                if (i == 0)
                    return p;

                // Otherwise, if we *can* see a processor, decide by its IO/MIDI flags
                if (auto* proc = ext->getAudioPluginInstance())
                    if ((proc->acceptsMidi() || proc->getNumInputChannels() == 0)
                         && proc->getTotalNumOutputChannels() > 0
                         && !proc->isMidiEffect())
                        return p;
            }
        }
    DBG("[INST] No instrument found; chain is:");
    for (int i = 0; i < track.pluginList.size(); ++i)
        if (auto* p = track.pluginList[i])
            DBG("  slot " << i << ": " << p->getName());
    return nullptr;
}

static juce::Component* createInstrumentEditorComponent (te::Plugin& p)
{

    // External VST3/AU → use the plugin's AudioProcessor editor
    if (auto* ext = dynamic_cast<te::ExternalPlugin*> (&p))
        if (auto* inst = ext->getAudioPluginInstance())
            if (auto* ed = inst->createEditorIfNeeded())
                return ed; // caller will own it via PluginEditorWindow
            else
                return new juce::GenericAudioProcessorEditor (*inst);

    return nullptr; // unknown plugin type
}


void AppEngine::instrumentButtonPressed (te::AudioTrack& track,
                                         juce::Component& anchor,
                                         std::function<void()> onChanged = {})
{
    if (auto* inst = findInstrument (track))
    {
        // Open the plugin UI/editor
        // Preferred: use UIBehaviour if available in your TE build:
        // engine->getUIBehaviour().showPluginWindow (inst);

        // Generic external editor fallback (covers VST3/AU):
        if (auto* ext = dynamic_cast<te::ExternalPlugin*> (inst))
        {
            if (auto* proc = ext->getAudioPluginInstance())
            {
                if (auto* ed = proc->createEditorIfNeeded())
                {
                    struct HostWindow : juce::DocumentWindow
                    {
                        HostWindow (juce::Component* c) :
                            juce::DocumentWindow (c->getName(), juce::Colours::black,
                                                  juce::DocumentWindow::closeButton)
                        {
                            setUsingNativeTitleBar (true);
                            setContentOwned (c, true);
                            centreWithSize (getWidth(), getHeight());
                            setVisible (true);
                        }
                        void closeButtonPressed() override { delete this; }
                    };
                    new HostWindow (ed);
                    return;
                }
                // fallback to generic editor if plugin has no custom editor
                auto* generic = new juce::GenericAudioProcessorEditor (*proc);
                struct HostWindow2 : juce::DocumentWindow
                {
                    HostWindow2 (juce::Component* c) :
                        juce::DocumentWindow (c->getName(), juce::Colours::black,
                                              juce::DocumentWindow::closeButton)
                    {
                        setUsingNativeTitleBar (true);
                        setContentOwned (c, true);
                        centreWithSize (getWidth(), getHeight());
                        setVisible (true);
                    }
                    void closeButtonPressed() override { delete this; }
                };
                new HostWindow2 (generic);
                return;
            }
        }

        // If it's a built-in Tracktion instrument (e.g., FourOSC/Sampler) and your
        // TE build supports it, uncomment this instead of the generic path above:
        // engine->getUIBehaviour().showPluginWindow (inst);

        return;
    }

    // No instrument yet → show chooser
    showInstrumentMenuForTrack (track, anchor, std::move (onChanged));
}
