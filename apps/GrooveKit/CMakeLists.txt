juce_add_gui_app(GrooveKit PRODUCT_NAME "GrooveKit")

target_sources(GrooveKit PRIVATE main.cpp)

target_compile_definitions(GrooveKit PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        TRACKTION_ENABLE_ARA=0
        JUCE_PLUGINHOST_VST3=1
)

target_link_libraries(GrooveKit
        PRIVATE
        app_ui
        app_engine
        tracktion_engine
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
