juce_add_gui_app(GrooveKit
    PRODUCT_NAME "GrooveKit"
    ICON_BIG "${CMAKE_SOURCE_DIR}/src/resources/Icons/GrooveKit.icns"
)

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
        tracktion_engine
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Copy custom icon to app bundle after build (macOS)
if(APPLE)
    add_custom_command(TARGET GrooveKit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/src/resources/Icons/GrooveKit.icns"
            "$<TARGET_BUNDLE_CONTENT_DIR:GrooveKit>/Resources/Icon.icns"
        COMMENT "Copying custom icon to app bundle"
    )
endif()
