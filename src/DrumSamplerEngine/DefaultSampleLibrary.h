#pragma once
#include <juce_core/juce_core.h>

namespace DefaultSampleLibrary
{
    juce::File installRoot();

    void ensureInstalled();

    juce::Array<juce::File> listAll();
}