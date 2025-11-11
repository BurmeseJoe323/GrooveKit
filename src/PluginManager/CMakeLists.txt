add_library(plugin_manager STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.cpp
        PluginEditorWindow.h
        PluginEditorWindow.cpp
)

target_include_directories(plugin_manager
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(plugin_manager
        PUBLIC
        juce::juce_audio_processors
        tracktion_engine
)

target_compile_definitions(plugin_manager
        PUBLIC
        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_AU=1   # macOS only; harmless elsewhere
)

