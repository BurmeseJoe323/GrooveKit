#pragma once

#include <juce_core/juce_core.h>

/**
 * @brief Helpers for managing GrooveKit's default sample library on disk.
 *
 * This namespace provides:
 *  - A root directory for bundled + user-imported samples.
 *  - A one-time installer that writes BinaryData samples to disk.
 *  - A function to enumerate all installed sample files.
 */
namespace DefaultSampleLibrary
{
    /**
     * @brief Returns the root directory where default samples are installed.
     *
     * Creates the "GrooveKit/Samples" directory under the user's application
     * data folder if it doesn't already exist.
     */
    juce::File installRoot();

    /**
     * @brief Ensures the default sample library is installed.
     *
     * Copies any embedded BinaryData samples to the installRoot() directory,
     * organizing them into subfolders (Kicks, Snares, HiHats, Toms, UserImports).
     * Also migrates any "flat" wavs in the root into categorized subfolders.
     */
    void ensureInstalled();

    /**
     * @brief Lists all sample files in the default sample library.
     *
     * Recursively scans installRoot() for .wav files.
     *
     * @return Array of files found under the sample root.
     */
    juce::Array<juce::File> listAll();
}
