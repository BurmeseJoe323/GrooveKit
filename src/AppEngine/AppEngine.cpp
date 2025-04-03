
#include "AppEngine.h"
#include <tracktion_engine/tracktion_engine.h>



namespace te = tracktion;
using namespace std::literals;
using namespace te::literals;

AppEngine::AppEngine()
{
    engine = std::make_unique<te::Engine>("GrooveKitEngine");

    createOrLoadEdit();


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
void AppEngine::addMidiTrackAndClip()
{
    edit->ensureNumberOfAudioTracks(1);
    auto track = te::getAudioTracks(*edit)[0];

    // Insert a MIDI clip that lasts one bar
    te::TimeRange oneBar(0s, edit->tempoSequence.toTime({1, te::BeatDuration()}));

    auto clip = track->insertNewClip(te::TrackItem::Type::midi, "Midi Clip", oneBar, nullptr);
    auto midiClip = dynamic_cast<te::MidiClip*>(clip);

    if (midiClip == nullptr)
        return;

    // Add notes to the MIDI clip
    midiClip->getSequence().addNote(60, 0_bp, 0.5_bd, 100, 0, nullptr); // C
    midiClip->getSequence().addNote(64, 1_bp, 0.5_bd, 100, 0, nullptr); // E
    midiClip->getSequence().addNote(67, 2_bp, 0.5_bd, 100, 0, nullptr); // G
    midiClip->getSequence().addNote(72, 3_bp, 0.5_bd, 100, 0, nullptr); // High C

    // Add a plugin to play the MIDI notes
    auto plugin = edit->getPluginCache()
                      .createNewPlugin(te::FourOscPlugin::xmlTypeName, {})
                      .get();
    if (plugin)
        track->pluginList.insertPlugin(*plugin, 0, nullptr);
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
    addMidiTrackAndClip();
    play();
}




