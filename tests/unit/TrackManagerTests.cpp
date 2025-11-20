#include <catch2/catch_test_macros.hpp>
#include "DrumSamplerEngine/DrumSamplerEngineAdapter.h"

TEST_CASE("Pad to MIDI note conversion", "[drum][midi]")
{
    SECTION("Valid pad indices map to correct MIDI notes")
    {
        // Pad 0 should map to MIDI note 36 (C1)
        REQUIRE(padToMidiNote(0) == 36);

        // Pad 1 should map to MIDI note 37 (C#1)
        REQUIRE(padToMidiNote(1) == 37);

        // Pad 7 should map to MIDI note 43 (G1)
        REQUIRE(padToMidiNote(7) == 43);

        // Pad 15 should map to MIDI note 51 (D#2) - highest valid pad
        REQUIRE(padToMidiNote(15) == 51);
    }

    SECTION("All 16 pads map to sequential MIDI notes")
    {
        for (int pad = 0; pad < 16; ++pad)
        {
            int expectedNote = 36 + pad;
            REQUIRE(padToMidiNote(pad) == expectedNote);
        }
    }

    SECTION("Negative pad indices are clamped to pad 0")
    {
        REQUIRE(padToMidiNote(-1) == 36);
        REQUIRE(padToMidiNote(-5) == 36);
        REQUIRE(padToMidiNote(-100) == 36);
    }

    SECTION("Pad indices above 15 are clamped to pad 15")
    {
        REQUIRE(padToMidiNote(16) == 51);
        REQUIRE(padToMidiNote(20) == 51);
        REQUIRE(padToMidiNote(100) == 51);
    }

    SECTION("MIDI note range is correct (C1 to D#2)")
    {
        // Verify the entire range is 36-51
        int minNote = padToMidiNote(0);
        int maxNote = padToMidiNote(15);

        REQUIRE(minNote == 36);  // C1
        REQUIRE(maxNote == 51);  // D#2
        REQUIRE(maxNote - minNote == 15);  // 16 notes total
    }
}

// NOTE: Full TrackManager tests require integration testing with Tracktion Engine
// These tests would belong in tests/integration/ and would need:
// - te::Engine instance setup
// - te::Edit instance creation
// - Proper audio device initialization
//
// Example integration tests to implement later:
// - TEST_CASE("TrackManager adding drum tracks")
// - TEST_CASE("TrackManager adding instrument tracks")
// - TEST_CASE("TrackManager track type persistence")
// - TEST_CASE("TrackManager solo/mute behavior")
