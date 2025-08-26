add_library(midi_engine)
target_sources(midi_engine PRIVATE MIDIEngine.cpp PUBLIC MIDIEngine.h)
target_include_directories(midi_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(midi_engine
        PUBLIC
        juce::juce_audio_devices
        juce::juce_audio_basics
        tracktion_engine
)
