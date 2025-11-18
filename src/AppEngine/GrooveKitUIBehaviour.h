#pragma once

#include <tracktion_engine/tracktion_engine.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace te = tracktion::engine;

class GrooveKitUIBehaviour : public te::UIBehaviour
{
public:
    GrooveKitUIBehaviour() = default;

    void runTaskWithProgressBar (te::ThreadPoolJobWithProgress& job) override
    {
        while (job.runJob() == juce::ThreadPoolJob::jobNeedsRunningAgain)
        {
            //juce::MessageManager::getInstance()->runDispatchLoopUntil (10);
        }
    }
};
