#pragma once

#include <string>
#include <regex>
#include <cmath>
#include <juce_core/juce_core.h>

/**
 * @brief Utility functions for validating and constraining user input in UI components.
 *
 * ValidationUtils provides helper functions for common validation tasks in GrooveKit:
 *  - Numeric string validation (integer and decimal formats)
 *  - BPM range constraining and rounding
 *
 * Architecture:
 *  - Header-only namespace with inline functions
 *  - Used by TransportBar for BPM input validation
 *  - Uses C++ regex for string parsing
 *  - Uses JUCE jlimit() for range constraining
 *
 * Usage:
 *  - Call isValidNumeric() before parsing user-entered numeric text
 *  - Call constrainAndRoundBpm() to ensure BPM values stay within valid range (20-250)
 *  - All functions are thread-safe (no state)
 */
namespace ValidationUtils
{
    //==============================================================================
    // Numeric Validation

    /**
     * @brief Validates if a string contains a valid numeric value (integer or decimal).
     *
     * Accepts formats: "123", "123.45", ".45"
     * Rejects formats: "abc", "12.34.56", "-10", ""
     *
     * @param text The string to validate
     * @return true if the string is a valid number, false otherwise
     */
    inline bool isValidNumeric(const std::string& text)
    {
        return std::regex_match(text, std::regex("^\\d+\\.?\\d*$|^\\.\\d+$"));
    }

    //==============================================================================
    // BPM Validation

    /**
     * @brief Constrains and rounds a BPM value to valid range and precision.
     *
     * Applies two transformations:
     *  1. Constrains to [minBpm, maxBpm] range using JUCE jlimit()
     *  2. Rounds to specified decimal places for cleaner display
     *
     * @param value The BPM value to process
     * @param minBpm Minimum allowed BPM (default: 20.0)
     * @param maxBpm Maximum allowed BPM (default: 250.0)
     * @param decimalPlaces Number of decimal places to round to (default: 2)
     * @return Constrained and rounded BPM value
     */
    inline double constrainAndRoundBpm(double value,
                                       double minBpm = 20.0,
                                       double maxBpm = 250.0,
                                       int decimalPlaces = 2)
    {
        // Constrain to valid range
        double constrainedValue = juce::jlimit (minBpm, maxBpm, value);

        // Round to specified decimal places
        double multiplier = std::pow(10.0, decimalPlaces);
        return std::round(constrainedValue * multiplier) / multiplier;
    }
}
