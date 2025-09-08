
#include "AppEngine.h"
#include <tracktion_engine/tracktion_engine.h>



namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

AppEngine::AppEngine()
{
    engine = std::make_unique<te::Engine>("GrooveKitEngine");

    createOrLoadEdit();

    midiEngine = std::make_unique<MIDIEngine>(*edit);
    audioEngine = std::make_unique<AudioEngine>(*edit);
    trackManager = std::make_unique<TrackManager>(*edit); 
    selectionManager = std::make_unique<te::SelectionManager>(*engine);

    editViewState = std::make_unique<EditViewState>(*edit, *selectionManager);
}

AppEngine::~AppEngine() = default;


void AppEngine::createOrLoadEdit()
{
    auto editFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
                        .getChildFile("GrooveKit")
                        .getNonexistentChildFile("Test", ".tracktionedit", false);

    if (editFile.existsAsFile())
        edit = te::loadEditFromFile(*engine, editFile);

    else
        edit = te::createEmptyEdit(*engine, editFile);

    edit->editFileRetriever = [editFile] { return editFile; };
    edit->playInStopEnabled = true;

    edit->restartPlayback();
}

void AppEngine::play() { audioEngine->play(); }

void AppEngine::stop() { audioEngine->stop(); }

int AppEngine::addMidiTrack() { return trackManager->addTrack(); }

void AppEngine::deleteMidiTrack(int index) { trackManager->deleteTrack(index); }

void AppEngine::addMidiClipToTrack(int trackIndex) { midiEngine->addMidiClipToTrack(trackIndex); }

te::MidiList &AppEngine::getMidiClipFromTrack(int trackIndex) {
    return midiEngine->getMidiClipFromTrack(trackIndex);
}

int AppEngine::getNumTracks() { return tracktion::getAudioTracks(*edit).size(); }

EditViewState &AppEngine::getEditViewState() { return *editViewState; }

te::Edit & AppEngine::getEdit() { return *edit; }

void AppEngine::soloTrack(int i)                 { trackManager->soloTrack(i); }
void AppEngine::setTrackSoloed(int i, bool s)    { trackManager->setTrackSoloed(i, s); }
bool AppEngine::isTrackSoloed(int i) const       { return trackManager->isTrackSoloed(i); }
bool AppEngine::anyTrackSoloed() const           { return trackManager->anyTrackSoloed(); }
