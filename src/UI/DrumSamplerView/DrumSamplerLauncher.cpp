#include "DrumSamplerLauncher.h"
#include "DrumSamplerView.h"  // JIVE headers are only included here

std::unique_ptr<juce::Component> makeDrumSamplerView(AudioEngine& audio, MIDIEngine& midi)
{
    return std::make_unique<DrumSamplerView>(audio, midi);
}
