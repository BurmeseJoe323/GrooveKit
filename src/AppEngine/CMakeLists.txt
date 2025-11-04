add_library(app_engine
        ../PluginManager/PluginManager.h
        ../PluginManager/PluginManager.cpp)
target_sources(app_engine PRIVATE
        AppEngine.cpp
        TrackManager.cpp
        ${CMAKE_SOURCE_DIR}/src/UI/Plugins/Synthesizer/MorphSynthPlugin.cpp
        ${CMAKE_SOURCE_DIR}/src/UI/Plugins/Synthesizer/MorphSynthPlugin.h
        ${CMAKE_SOURCE_DIR}/src/UI/Plugins/Synthesizer/MorphVoice.h
        ${CMAKE_SOURCE_DIR}/src/UI/Plugins/Synthesizer/MorphOsc.h
        ${CMAKE_SOURCE_DIR}/src/UI/Plugins/Synthesizer/MorphSynthRegistration.h
)
target_sources(app_engine PUBLIC AppEngine.h TrackManager.h)
target_include_directories(app_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app_engine
        PUBLIC
        tracktion_engine
        midi_engine
        audio_engine
        drum_sampler_engine
        GrooveKitResources
)
