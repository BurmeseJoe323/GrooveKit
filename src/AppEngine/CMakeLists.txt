add_library(app_engine
        ../PluginManager/PluginManager.h
        ../PluginManager/PluginManager.cpp)
target_sources(app_engine PRIVATE AppEngine.cpp MidiListener.cpp PUBLIC AppEngine.h TrackManager.h MidiListener.h TrackManager.cpp)
target_include_directories(app_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app_engine
        PUBLIC
        tracktion_engine
        midi_engine
        audio_engine
        drum_sampler_engine
        GrooveKitResources
)
