add_library(audio_engine)
target_sources(audio_engine PRIVATE AudioEngine.cpp PUBLIC AudioEngine.h)
target_include_directories(audio_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# also inherit tracktion_engine's include dirs
target_link_libraries(audio_engine
        PUBLIC
        juce::juce_audio_processors
        tracktion_engine
)
