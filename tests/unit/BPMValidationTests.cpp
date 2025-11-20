#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "AppEngine/ValidationUtils.h"

TEST_CASE("Numeric validation", "[validation][bpm]")
{
    SECTION("Valid integer strings")
    {
        REQUIRE(ValidationUtils::isValidNumeric("120"));
        REQUIRE(ValidationUtils::isValidNumeric("0"));
        REQUIRE(ValidationUtils::isValidNumeric("999"));
    }

    SECTION("Valid decimal strings")
    {
        REQUIRE(ValidationUtils::isValidNumeric("120.5"));
        REQUIRE(ValidationUtils::isValidNumeric("0.0"));
        REQUIRE(ValidationUtils::isValidNumeric("138.75"));
        REQUIRE(ValidationUtils::isValidNumeric(".5"));  // Leading decimal
        REQUIRE(ValidationUtils::isValidNumeric("120."));  // Trailing decimal
    }

    SECTION("Invalid strings")
    {
        REQUIRE_FALSE(ValidationUtils::isValidNumeric("abc"));
        REQUIRE_FALSE(ValidationUtils::isValidNumeric("12a"));
        REQUIRE_FALSE(ValidationUtils::isValidNumeric("1.2.3"));
        REQUIRE_FALSE(ValidationUtils::isValidNumeric("-120"));  // Negative not allowed
        REQUIRE_FALSE(ValidationUtils::isValidNumeric(""));
        REQUIRE_FALSE(ValidationUtils::isValidNumeric(" "));
        REQUIRE_FALSE(ValidationUtils::isValidNumeric("12 0"));  // Space in middle
    }

    SECTION("Edge cases")
    {
        REQUIRE(ValidationUtils::isValidNumeric("000"));
        REQUIRE(ValidationUtils::isValidNumeric("0.000"));
    }
}

TEST_CASE("BPM constraint and rounding", "[validation][bpm]")
{
    SECTION("Values within valid range are preserved")
    {
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.0) == Catch::Approx(120.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(138.5) == Catch::Approx(138.5));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(75.25) == Catch::Approx(75.25));
    }

    SECTION("Values below minimum are clamped to 20")
    {
        REQUIRE(ValidationUtils::constrainAndRoundBpm(0.0) == Catch::Approx(20.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(10.0) == Catch::Approx(20.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(19.99) == Catch::Approx(20.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(-50.0) == Catch::Approx(20.0));
    }

    SECTION("Values above maximum are clamped to 250")
    {
        REQUIRE(ValidationUtils::constrainAndRoundBpm(300.0) == Catch::Approx(250.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(500.0) == Catch::Approx(250.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(250.01) == Catch::Approx(250.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(1000.0) == Catch::Approx(250.0));
    }

    SECTION("Boundary values")
    {
        REQUIRE(ValidationUtils::constrainAndRoundBpm(20.0) == Catch::Approx(20.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(250.0) == Catch::Approx(250.0));
    }

    SECTION("Rounding to 2 decimal places")
    {
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.123) == Catch::Approx(120.12));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.125) == Catch::Approx(120.13));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.999) == Catch::Approx(121.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(138.555) == Catch::Approx(138.56));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(75.004) == Catch::Approx(75.0));
    }

    SECTION("Combined constraints and rounding")
    {
        // Below minimum with rounding
        REQUIRE(ValidationUtils::constrainAndRoundBpm(15.999) == Catch::Approx(20.0));

        // Above maximum with rounding
        REQUIRE(ValidationUtils::constrainAndRoundBpm(255.555) == Catch::Approx(250.0));

        // Valid range with rounding
        REQUIRE(ValidationUtils::constrainAndRoundBpm(128.456) == Catch::Approx(128.46));
    }
}

TEST_CASE("BPM validation with custom parameters", "[validation][bpm]")
{
    SECTION("Custom min/max range")
    {
        REQUIRE(ValidationUtils::constrainAndRoundBpm(50.0, 60.0, 180.0) == Catch::Approx(60.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(200.0, 60.0, 180.0) == Catch::Approx(180.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.0, 60.0, 180.0) == Catch::Approx(120.0));
    }

    SECTION("Custom decimal places")
    {
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.456, 20.0, 250.0, 0) == Catch::Approx(120.0));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.456, 20.0, 250.0, 1) == Catch::Approx(120.5));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.456, 20.0, 250.0, 2) == Catch::Approx(120.46));
        REQUIRE(ValidationUtils::constrainAndRoundBpm(120.456, 20.0, 250.0, 3) == Catch::Approx(120.456));
    }
}
