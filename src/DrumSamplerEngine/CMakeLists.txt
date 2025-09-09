# src/DrumSamplerEngine/CMakeLists.txt
add_library(drum_sampler_engine STATIC
        DrumSamplerEngineAdapter.cpp
        DrumSamplerEngineAdapter.h
)

# The adapter includes your UI-facing interface:
#   #include "DrumSamplerEngine.h"
# which lives in src/UI/DrumSamplerView
target_include_directories(drum_sampler_engine
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_SOURCE_DIR}/src/UI/DrumSamplerView
)

# Link Tracktion so its INTERFACE include dirs propagate to dependents
target_link_libraries(drum_sampler_engine
        PUBLIC
        tracktion_engine
        tracktion_graph
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_audio_basics
        juce::juce_core
)
