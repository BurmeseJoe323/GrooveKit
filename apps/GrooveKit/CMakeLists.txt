juce_add_gui_app(GrooveKit PRODUCT_NAME "GrooveKit")

target_sources(GrooveKit PRIVATE
        main.cpp
        ${CMAKE_SOURCE_DIR}/src/PluginManager/PluginManager.cpp
        ${CMAKE_SOURCE_DIR}/src/PluginManager/PluginManager.h
)

target_compile_definitions(GrooveKit PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        TRACKTION_ENABLE_ARA=0
        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_AU=1
        JUCE_MODAL_LOOPS_PERMITTED=1
)

if(APPLE)
    target_compile_definitions(GrooveKit PRIVATE JUCE_PLUGINHOST_AU=1)
endif()

target_link_libraries(GrooveKit
        PRIVATE
        app_ui
        app_engine
        database_manager
        juce::juce_audio_processors
        tracktion_engine
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
