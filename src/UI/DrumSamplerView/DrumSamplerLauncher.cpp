#include "DrumSamplerView.h"
#include "DrumSamplerEngine.h"
#include "../AudioEngine/AudioEngine.h"
#include "../MIDIEngine/MIDIEngine.h"
#include <juce_core/juce_core.h>

namespace
{
    // TEMP adapter — replace with your real Tracktion-backed engine when ready.
    struct NoopDrumSamplerEngine : DrumSamplerEngine
    {
        juce::String slotNames[16];

        void loadSampleIntoSlot (int slot, const juce::File& file) override
        {
            slot = juce::jlimit (0, 15, slot);
            slotNames[slot] = file.getFileNameWithoutExtension();
            juce::Logger::outputDebugString ("Loaded into slot " + juce::String (slot) + ": " + file.getFullPathName());
        }

        void triggerSlot (int slot, float velocity) override
        {
            juce::ignoreUnused (velocity);
            juce::Logger::outputDebugString ("Trigger slot " + juce::String (slot));
        }

        void setVolume (float v) override
        {
            juce::Logger::outputDebugString ("Volume " + juce::String (v));
        }

        void setADSR (float a, float d, float s, float r) override
        {
            juce::Logger::outputDebugString ("ADSR " + juce::String (a) + "," + juce::String (d) + "," + juce::String (s) + "," + juce::String (r));
        }

        juce::String getSlotName (int slot) const override
        {
            slot = juce::jlimit (0, 15, slot);
            return slotNames[slot].isNotEmpty() ? slotNames[slot] : ("Pad " + juce::String (slot + 1));
        }
    };
}

// Your project’s factory signature (adjust include path if needed)
std::unique_ptr<juce::Component> makeDrumSamplerView (AudioEngine& audio, MIDIEngine& midi)
{
    juce::ignoreUnused (audio, midi);

    static NoopDrumSamplerEngine engine; // keep alive for the window's lifetime
    return std::make_unique<DrumSamplerView> (engine);
}
