
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
    if (edit)
    {
        auto& transport = edit->getTransport();
        transport.setPosition(0s);
        transport.setLoopRange(tracktion::TimeRange::between(0s, 4s));
        transport.looping = true;
        transport.play(false);
    }
}
void AppEngine::stop()
{
    if (edit)
    {
        edit->getTransport().stop(false, false);
    }
}

void AppEngine::start()
{
    if (midiEngine) {
        midiEngine->addMidiTrack();
        midiEngine->addMidiClipToTrack(0);
    }
    play();
}

void AppEngine::addMidiTrack()
{
    midiEngine->addMidiTrack();
}

void AppEngine::addMidiClipToTrack(int trackIndex)
{
    midiEngine->addMidiClipToTrack(trackIndex);
}