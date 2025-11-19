#pragma once

#include <memory>
#include <juce_gui_basics/juce_gui_basics.h>

class AudioEngine;
class MIDIEngine;

/**
 * @brief Factory function to create the Drum Sampler UI view.
 *
 * This returns a fully constructed DrumSamplerView component, which is the
 * top-level UI for the sampler (pads grid, ADSR, volume, sample library).
 *
 * The AudioEngine and MIDIEngine references are passed in to allow future
 * integration with playback/routing if needed. Currently, the launcher
 * constructs a temporary internal engine implementation for UI-only usage.
 *
 * @param audio Reference to the application's AudioEngine.
 * @param midi  Reference to the application's MIDIEngine.
 *
 * @return std::unique_ptr<juce::Component> Owning pointer to the sampler UI.
 */
std::unique_ptr<juce::Component> makeDrumSamplerView (AudioEngine& audio,
                                                      MIDIEngine& midi);