cmake_minimum_required(VERSION 3.15)
project(GrooveKit VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Load paths
include(${CMAKE_SOURCE_DIR}/cmake/JUCEPath.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/TracktionPath.cmake)

# Tracktion-bundled JUCE
juce_add_modules(${TRACKTION_ENGINE_PATH}/modules/tracktion_graph)
juce_add_modules(${TRACKTION_ENGINE_PATH}/modules/tracktion_engine)

# libraries
add_subdirectory(src/AppEngine)
add_subdirectory(src/MIDIEngine)
add_subdirectory(src/AudioEngine)
add_subdirectory(src/UI)
add_subdirectory(src/DatabaseManager)

# Apps
add_subdirectory(apps/GrooveKit)
