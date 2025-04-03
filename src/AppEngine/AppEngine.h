#pragma once

#include <tracktion_engine/tracktion_engine.h>

class AppEngine
{
public:
    AppEngine();
    ~AppEngine();

    void createOrLoadEdit();
    void addMidiTrackAndClip();
    void play();
    void stop();
    void start();

private:
    std::unique_ptr<tracktion::engine::Engine> engine;
    std::unique_ptr<tracktion::engine::Edit> edit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppEngine)
};