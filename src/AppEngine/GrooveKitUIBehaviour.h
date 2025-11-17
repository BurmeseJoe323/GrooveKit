#pragma once

#include <tracktion_engine/tracktion_engine.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace te = tracktion::engine;

// Minimal behaviour: just run the job to completion, no fancy UI yet.
class GrooveKitUIBehaviour : public te::UIBehaviour
{
public:
    GrooveKitUIBehaviour() = default;

    void runTaskWithProgressBar (te::ThreadPoolJobWithProgress& job) override
    {
        // This satisfies UIBehaviour and stops the jassert.
        // If you want to show a modal overlay later, we can extend this.
        while (job.runJob() == juce::ThreadPoolJob::jobNeedsRunningAgain)
        {
            //juce::MessageManager::getInstance()->runDispatchLoopUntil (10);
            // Keep the UI responsive while rendering
            juce::MessageManager::getInstance()->runDispatchLoopUntil (10);
        }
    }
};
