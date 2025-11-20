#include "DrumSamplerView.h"
#include "DrumSamplerEngine.h"
#include "../AudioEngine/AudioEngine.h"
#include "../MIDIEngine/MIDIEngine.h"
#include <juce_core/juce_core.h>

/**
 * @file DrumSamplerLauncher.cpp
 *
 * @brief Creates the DrumSamplerView and provides a simple internal
 *        test engine (NoopDrumSamplerEngine) for driving the UI.
 *
 * This file does not contain real audio functionality — it simply logs
 * events. The real engine (DrumSamplerEngineAdapter) can be substituted
 * here without changing UI code.
 */

namespace
{
    /**
     * @brief Minimal stand-in implementation of DrumSamplerEngine.
     *
     * This engine:
     *   - Stores sample names per slot (Pads 1–16)
     *   - Logs triggers, volume, and ADSR changes
     *
     * This allows the UI to function even when the real Tracktion-based
     * drum engine is not yet connected.
     */
    struct NoopDrumSamplerEngine : DrumSamplerEngine
    {
        juce::String slotNames[16];

        void loadSampleIntoSlot (int slot, const juce::File& file) override
        {
            slot = juce::jlimit (0, 15, slot);
            slotNames[slot] = file.getFileNameWithoutExtension();

            juce::Logger::outputDebugString (
                "Loaded into slot " + juce::String (slot)
                + ": " + file.getFullPathName());
        }

        void triggerSlot (int slot, float velocity) override
        {
            juce::ignoreUnused (velocity);

            juce::Logger::outputDebugString (
                "Trigger slot " + juce::String (slot));
        }

        void setVolume (float v) override
        {
            juce::Logger::outputDebugString (
                "Volume " + juce::String (v));
        }

        void setADSR (float a, float d, float s, float r) override
        {
            juce::Logger::outputDebugString (
                "ADSR " + juce::String (a) + ","
                        + juce::String (d) + ","
                        + juce::String (s) + ","
                        + juce::String (r));
        }

        juce::String getSlotName (int slot) const override
        {
            slot = juce::jlimit (0, 15, slot);

            return slotNames[slot].isNotEmpty()
                    ? slotNames[slot]
                    : ("Pad " + juce::String (slot + 1));
        }
    };
} // namespace

/**
 * @brief Factory for constructing the full DrumSamplerView.
 *
 * Currently uses NoopDrumSamplerEngine as backend. Later, this should be
 * replaced with DrumSamplerEngineAdapter so the UI is connected to actual
 * Tracktion playback.
 *
 * @param audio The AudioEngine (unused for now).
 * @param midi  The MIDIEngine (unused for now).
 *
 * @return A new DrumSamplerView wrapped in a unique_ptr.
 */
std::unique_ptr<juce::Component> makeDrumSamplerView (AudioEngine& audio,
                                                      MIDIEngine& midi)
{
    juce::ignoreUnused (audio, midi);

    static NoopDrumSamplerEngine engine;  // Persistent engine instance for UI
    return std::make_unique<DrumSamplerView> (engine);
}
