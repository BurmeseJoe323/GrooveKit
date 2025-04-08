
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
    trackManager = std::make_unique<TrackManager>(*edit);  // ✅ Add this line

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

void AppEngine::play()
{
    audioEngine->play();
}
void AppEngine::stop()
{
    audioEngine->stop();
}

void AppEngine::addMidiTrack()
{
    juce::Logger::outputDebugString("Track added");
    trackManager->addTrack();
}

void AppEngine::deleteMidiTrack(int index) {
    trackManager->deleteTrack(index);
}

void AppEngine::addMidiClipToTrack(int trackIndex)
{
    midiEngine->addMidiClipToTrack(trackIndex);
}