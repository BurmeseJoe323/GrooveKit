#pragma once

#include <string>
#include <regex>
#include <cmath>
#include <juce_core/juce_core.h>

namespace ValidationUtils
{
    /**
     * Validates if a string contains a valid numeric value (integer or decimal)
     * @param text The string to validate
     * @return true if the string is a valid number, false otherwise
     */
    inline bool isValidNumeric(const std::string& text)
    {
        return std::regex_match(text, std::regex("^\\d+\\.?\\d*$|^\\.\\d+$"));
    }

    /**
     * Constrains and rounds a BPM value to valid range and precision
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
