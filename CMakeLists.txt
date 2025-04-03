cmake_minimum_required(VERSION 3.15)
project(GrooveKit VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Load JUCE path
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/JUCEPath.cmake")
    include("${CMAKE_SOURCE_DIR}/cmake/JUCEPath.cmake")
else()
    message(FATAL_ERROR "JUCEPath.cmake not found. Please create it and specify your JUCE directory.")
endif()

# Load Tracktion Engine path
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/TracktionPath.cmake")
    include("${CMAKE_SOURCE_DIR}/cmake/TracktionPath.cmake")
else()
    message(FATAL_ERROR "TracktionPath.cmake not found. Please create it and specify your TRACKTION_ENGINE_PATH.")
endif()

# Add Tracktion-bundled JUCE as subdirectory
add_subdirectory(${TRACKTION_ENGINE_PATH}/modules/juce ${CMAKE_BINARY_DIR}/juce)

# Register tracktion_graph before tracktion_engine
juce_add_modules(${TRACKTION_ENGINE_PATH}/modules/tracktion_graph)

# Add Tracktion Engine
juce_add_modules(${TRACKTION_ENGINE_PATH}/modules/tracktion_engine)

# build based on platform
if (WIN32)
    #Windows: Build as GUI app (uses WinMain via JUCE)
    add_executable(GrooveKit WIN32
            src/main.cpp
            src/MainComponent.h
            src/MainComponent.cpp
    )
elseif (APPLE)
    # macOS: Build as console app (uses main via JUCE)
    add_executable(GrooveKit
            src/main.cpp
            src/MainComponent.h
            src/MainComponent.cpp
    )
endif()

# Compile definitions
target_compile_definitions(GrooveKit PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        TRACKTION_ENABLE_ARA=0
        JUCE_PLUGINHOST_VST3=1
)

# Link to JUCE and Tracktion Engine
target_link_libraries(GrooveKit PRIVATE
        tracktion_engine

        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
