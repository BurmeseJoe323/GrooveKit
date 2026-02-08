cmake_minimum_required(VERSION 3.15)
project(GrooveKit VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Tracktion Engine from third-party/ (includes JUCE as submodule)
set(TRACKTION_ENGINE_PATH "${CMAKE_SOURCE_DIR}/third-party/tracktion_engine" CACHE PATH "Path to Tracktion Engine")

add_subdirectory(${TRACKTION_ENGINE_PATH}/modules/juce ${CMAKE_BINARY_DIR}/juce)

# Tracktion-bundled JUCE
juce_add_modules(
        ${TRACKTION_ENGINE_PATH}/modules/tracktion_graph
        ${TRACKTION_ENGINE_PATH}/modules/tracktion_engine
)

# Melatonin Inspector
Include (FetchContent)
FetchContent_Declare (melatonin_inspector
        GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
        GIT_TAG origin/main
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_inspector)
FetchContent_MakeAvailable (melatonin_inspector)

file(GLOB_RECURSE GROOVEKIT_DEFAULT_WAVS
        ${CMAKE_SOURCE_DIR}/src/resources/Samples/*.[Ww][Aa][Vv]
)

list(LENGTH GROOVEKIT_DEFAULT_WAVS GROOVEKIT_NUM_WAVS)
message(STATUS "GrooveKit: embedding ${GROOVEKIT_NUM_WAVS} default WAV(s)")
foreach(w ${GROOVEKIT_DEFAULT_WAVS})
    message(STATUS "  WAV: ${w}")
endforeach()

if(GROOVEKIT_NUM_WAVS EQUAL 0)
    message(WARNING "No WAVs found under src/resources/Samples; creating dummy resource so juce_add_binary_data has at least one file.")
    set(GROOVEKIT_DUMMY_RESOURCE "${CMAKE_CURRENT_BINARY_DIR}/__groovekit_dummy_resource__.txt")
    file(WRITE "${GROOVEKIT_DUMMY_RESOURCE}" "GrooveKit dummy resource.\n")
    set(GROOVEKIT_RESOURCE_FILES "${GROOVEKIT_DUMMY_RESOURCE}")
else()
    set(GROOVEKIT_RESOURCE_FILES ${GROOVEKIT_DEFAULT_WAVS})
endif()

juce_add_binary_data(GrooveKitResources
        SOURCES
        ${GROOVEKIT_RESOURCE_FILES}
)

target_sources(GrooveKitResources PRIVATE ${GROOVEKIT_DEFAULT_WAVS})

# ---- libraries
add_subdirectory(src/PluginManager)
add_subdirectory(src/AppEngine)
add_subdirectory(src/MIDIEngine)
add_subdirectory(src/AudioEngine)
add_subdirectory(src/DrumSamplerEngine)
add_subdirectory(src/UI)

# ---- App target (defines GrooveKit)
add_subdirectory(apps/GrooveKit)

# ---- Tests
option(BUILD_TESTING "Build the testing tree" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

target_compile_definitions(GrooveKit
        PRIVATE
        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_AU=1 # macOS only
)


# ---- Link external libs only (avoid duplicates)
target_link_libraries(GrooveKit
        PRIVATE
        melatonin_inspector
)
