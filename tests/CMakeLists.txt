# Catch2 Testing Framework
Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(Catch2)

# Test executable
add_executable(groovekit_tests
    unit/BPMValidationTests.cpp
    unit/TrackManagerTests.cpp
)

# Link against project libraries and Catch2
target_link_libraries(groovekit_tests
    PRIVATE
    app_ui
    Catch2::Catch2WithMain
)

# Include directories for accessing source headers
target_include_directories(groovekit_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Register tests with CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(groovekit_tests)
