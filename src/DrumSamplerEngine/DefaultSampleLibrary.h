#pragma once

#include <juce_core/juce_core.h>

/**
 * @brief Helpers for managing GrooveKit's default sample library on disk.
 *
 * The DefaultSampleLibrary namespace provides a small API for locating and
 * installing the bundled drum/sample content used by the DrumSampler and
 * related components.
 *
 * Typical usage:
 *  - Call ensureInstalled() during app startup to make sure the library is
 *    present in the user's application data directory.
 *  - Use installRoot() to find where the samples live on disk.
 *  - Use listAll() to enumerate all available sample files.
 */
namespace DefaultSampleLibrary
{
    //==============================================================================
    /**
     * @brief Returns the root directory where default samples are installed.
     *
     * This is usually a subdirectory of the user's application data or
     * documents folder, depending on the platform.
     *
     * @return juce::File representing the sample library root directory.
     */
    juce::File installRoot();

    /**
     * @brief Ensures the default sample library is installed on disk.
     *
     * If the library is not present at installRoot(), this function should
     * perform whatever copying or extraction is needed to populate the
     * directory with the built-in sample content.
     */
    void ensureInstalled();

    /**
     * @brief Lists all sample files in the default sample library.
     *
     * This should recursively or non-recursively enumerate the sample
     * directory (implementation-dependent) and return all discovered
     * sample files.
     *
     * @return Array of sample files available in the library.
     */
    juce::Array<juce::File> listAll();
}