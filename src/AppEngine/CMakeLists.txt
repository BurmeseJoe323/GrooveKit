add_library(app_engine)
target_sources(app_engine PRIVATE AppEngine.cpp PUBLIC AppEngine.h TrackManager.cpp TrackManager.h)
target_include_directories(app_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app_engine
        PUBLIC
        tracktion_engine
        midi_engine
        audio_engine
        drum_sampler_engine
        GrooveKitResources
)
