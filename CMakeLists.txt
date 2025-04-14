cmake_minimum_required(VERSION 3.15)
project(GrooveKit VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Load JUCE path
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/JUCEPath.cmake")
    include("${CMAKE_SOURCE_DIR}/cmake/JUCEPath.cmake")
else()
    message(FATAL_ERROR "JUCEPath.cmake not found. Please create it and specify your JUCE directory.")
endif()

# Load Tracktion Engine path
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/TracktionPath.cmake")
    include("${CMAKE_SOURCE_DIR}/cmake/TracktionPath.cmake")
else()
    message(FATAL_ERROR "TracktionPath.cmake not found. Please create it and specify your TRACKTION_ENGINE_PATH.")
endif()

# Add Tracktion-bundled JUCE as subdirectory
add_subdirectory(${TRACKTION_ENGINE_PATH}/modules/juce ${CMAKE_BINARY_DIR}/juce)

# Register tracktion_graph before tracktion_engine
juce_add_modules(${TRACKTION_ENGINE_PATH}/modules/tracktion_graph)

# Add Tracktion Engine
juce_add_modules(${TRACKTION_ENGINE_PATH}/modules/tracktion_engine)

# Add this somewhere before your add_executable
set(SQLITE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/database/sqlite3.c
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/database/sqlite3.h
        src/UI/InstrumentTutorial/InstrumentTutorial.cpp
        src/UI/InstrumentTutorial/InstrumentTutorial.h
)

# Build based on platform
if (WIN32)
    #Windows: Build as GUI app (uses WinMain via JUCE)
    add_executable(GrooveKit WIN32
            src/main.cpp
            src/UI/MainComponent.h
            src/UI/MainComponent.cpp
            src/UI/MainViews/AppView.h
            src/UI/TrackView-UI/TrackView.cpp
            src/UI/TrackView-UI/TrackView.h
            src/AppEngine/AppEngine.h
            src/AppEngine/AppEngine.cpp
            src/MIDIEngine/MIDIEngine.h
            src/MIDIEngine/MIDIEngine.cpp
            src/AudioEngine/AudioEngine.h
            src/AudioEngine/AudioEngine.cpp
            src/UI/MainViews/AppView.cpp
            src/UI/MainViews/WelcomeView.cpp
            src/UI/MainViews/WelcomeView.h
            src/UI/PopupWindows/PianoRollWindow.cpp
            src/UI/PopupWindows/PianoRollWindow.h
            src/UI/PopupWindows/PianoRollComponents/PianoRollEditor.cpp
            src/UI/PopupWindows/PianoRollComponents/PianoRollEditor.h
            src/UI/PopupWindows/PianoRollComponents/KeyboardComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/KeyboardComponent.h
            src/UI/PopupWindows/PianoRollComponents/NoteGridComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/NoteGridComponent.h
            src/UI/PopupWindows/PianoRollComponents/TimelineComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/TimelineComponent.h
            src/UI/PopupWindows/PianoRollComponents/GridControlPanel.cpp
            src/UI/PopupWindows/PianoRollComponents/GridControlPanel.h
            src/UI/PopupWindows/PianoRollComponents/GridStyleSheet.cpp
            src/UI/PopupWindows/PianoRollComponents/GridStyleSheet.h
            src/UI/PopupWindows/PianoRollComponents/PConstants.h
            src/UI/PopupWindows/PianoRollComponents/NoteComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/NoteComponent.h
            src/UI/PopupWindows/PianoRollComponents/NoteModel.cpp
            src/UI/PopupWindows/PianoRollComponents/NoteModel.h
            src/UI/PopupWindows/PianoRollComponents/PianoRollMainComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/PianoRollMainComponent.h
            src/UI/TrackView-UI/TrackComponent.cpp
            src/UI/TrackView-UI/TrackComponent.h
            src/UI/TrackView-UI/TrackHeader.cpp
            src/UI/TrackView-UI/TrackHeader.h
            Resources/exampleGUISetup.cpp
            Resources/exampleGUISetup.h
            src/UI/TrackView-UI/TrackClip.cpp
            src/UI/TrackView-UI/TrackClip.h
            src/UI/TrackView-UI/EditComponent.cpp
            src/UI/TrackView-UI/EditComponent.h
            src/AppEngine/TrackManager.h
            src/AppEngine/TrackManager.cpp
            ${SQLITE_SOURCES}
            src/UI/TrackView-Tutorial/TrackViewTut.cpp
            src/UI/TrackView-Tutorial/TrackViewTut.h
            src/DatabaseManager/DatabaseManager.cpp
            src/DatabaseManager/DatabaseManager.h
    )
elseif (APPLE)
    # macOS: Build as console app (uses main via JUCE)
    add_executable(GrooveKit
            src/main.cpp
            src/UI/MainComponent.h
            src/UI/MainComponent.cpp
            src/UI/MainViews/AppView.h
            src/UI/TrackView-UI/TrackView.cpp
            src/UI/TrackView-UI/TrackView.h
            src/AppEngine/AppEngine.h
            src/AppEngine/AppEngine.cpp
            src/MIDIEngine/MIDIEngine.h
            src/MIDIEngine/MIDIEngine.cpp
            src/AudioEngine/AudioEngine.h
            src/AudioEngine/AudioEngine.cpp
            src/UI/MainViews/AppView.cpp
            src/UI/MainViews/WelcomeView.cpp
            src/UI/MainViews/WelcomeView.h
            src/UI/PopupWindows/PianoRollWindow.cpp
            src/UI/PopupWindows/PianoRollWindow.h
            src/UI/PopupWindows/PianoRollComponents/PianoRollEditor.cpp
            src/UI/PopupWindows/PianoRollComponents/PianoRollEditor.h
            src/UI/PopupWindows/PianoRollComponents/KeyboardComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/KeyboardComponent.h
            src/UI/PopupWindows/PianoRollComponents/NoteGridComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/NoteGridComponent.h
            src/UI/PopupWindows/PianoRollComponents/TimelineComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/TimelineComponent.h
            src/UI/PopupWindows/PianoRollComponents/GridControlPanel.cpp
            src/UI/PopupWindows/PianoRollComponents/GridControlPanel.h
            src/UI/PopupWindows/PianoRollComponents/GridStyleSheet.cpp
            src/UI/PopupWindows/PianoRollComponents/GridStyleSheet.h
            src/UI/PopupWindows/PianoRollComponents/PConstants.h
            src/UI/PopupWindows/PianoRollComponents/NoteComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/NoteComponent.h
            src/UI/PopupWindows/PianoRollComponents/NoteModel.cpp
            src/UI/PopupWindows/PianoRollComponents/NoteModel.h
            src/UI/PopupWindows/PianoRollComponents/PianoRollMainComponent.cpp
            src/UI/PopupWindows/PianoRollComponents/PianoRollMainComponent.h
            src/UI/TrackView-UI/TrackComponent.cpp
            src/UI/TrackView-UI/TrackComponent.h
            src/UI/TrackView-UI/TrackHeader.cpp
            src/UI/TrackView-UI/TrackHeader.h
            Resources/exampleGUISetup.cpp
            Resources/exampleGUISetup.h
            src/UI/TrackView-UI/TrackClip.cpp
            src/UI/TrackView-UI/TrackClip.h
            src/UI/TrackView-UI/EditComponent.cpp
            src/UI/TrackView-UI/EditComponent.h
            src/AppEngine/TrackManager.h
            src/AppEngine/TrackManager.cpp
            ${SQLITE_SOURCES}
            src/UI/TrackView-Tutorial/TrackViewTut.cpp
            src/UI/TrackView-Tutorial/TrackViewTut.h
            src/DatabaseManager/DatabaseManager.cpp
            src/DatabaseManager/DatabaseManager.h
    )
endif()

# Compile definitions
target_compile_definitions(GrooveKit PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        TRACKTION_ENABLE_ARA=0
        JUCE_PLUGINHOST_VST3=1
)

# Link to JUCE and Tracktion Engine
target_link_libraries(GrooveKit PRIVATE
        tracktion_engine

        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Add include directory
target_include_directories(GrooveKit PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite
)
