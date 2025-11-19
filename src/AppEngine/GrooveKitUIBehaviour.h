#pragma once

#include <tracktion_engine/tracktion_engine.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace te = tracktion::engine;

/**
 * @brief Custom UI behaviour for GrooveKit's Tracktion Engine integration.
 *
 * This class overrides Tracktion Engine's UIBehaviour to control how
 * background tasks with progress are executed. In this implementation,
 * the job is run in a tight loop until completion without showing a
 * real progress bar or dispatching UI events.
 *
 * We can expand this later to:
 *  - Display an actual progress dialog.
 *  - Pump the JUCE message loop while the job is running.
 *  - Add cancellation support for long-running tasks.
 */
class GrooveKitUIBehaviour : public te::UIBehaviour
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Creates a new GrooveKitUIBehaviour instance.
     *
     * Uses the default UIBehaviour setup from Tracktion Engine and provides
     * a custom implementation for running jobs with progress.
     */
    GrooveKitUIBehaviour() = default;

    /** Default virtual destructor. */
    ~GrooveKitUIBehaviour() override = default;

    //==============================================================================
    // te::UIBehaviour Overrides

    /**
     * @brief Runs a ThreadPoolJobWithProgress until it finishes.
     *
     * This implementation simply runs the job repeatedly while it reports
     * that it still needs to run. No actual progress bar is displayed and
     * the JUCE message loop is not pumped here.
     *
     * @param job The job to be executed to completion.
     */
    void runTaskWithProgressBar (te::ThreadPoolJobWithProgress& job) override
    {
        while (job.runJob() == juce::ThreadPoolJob::jobNeedsRunningAgain)
        {
            // If needed later, you can pump the message loop here, e.g.:
            // juce::MessageManager::getInstance()->runDispatchLoopUntil (10);
        }
    }
};