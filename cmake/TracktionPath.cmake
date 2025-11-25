# TracktionPath.cmake
# Absolute path to your local Tracktion Engine checkout
set(TRACKTION_ENGINE_PATH "/Users/brackenasay/tracktion_engine" CACHE PATH "Path to Tracktion Engine" FORCE)

if (NOT EXISTS "${TRACKTION_ENGINE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "Tracktion Engine not found at: ${TRACKTION_ENGINE_PATH}")
endif()

# Add Tracktion as a subproject; it will add JUCE automatically
#add_subdirectory(${TRACKTION_ENGINE_PATH} ${CMAKE_BINARY_DIR}/tracktion_engine)