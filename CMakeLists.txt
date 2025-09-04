cmake_minimum_required(VERSION 3.15)
project(GrooveKit VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/cmake/JUCEPath.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/TracktionPath.cmake)

add_subdirectory(${TRACKTION_ENGINE_PATH}/modules/juce ${CMAKE_BINARY_DIR}/juce)

# --- JIVE via CPM (bootstrap CPM into the build dir) ---
file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
        EXPECTED_HASH SHA256=78ba32abdf798bc616bab7c73aac32a17bbd7b06ad9e26a6add69de8f3ae4791
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

# Either syntax works; this pins to the main branch:
CPMAddPackage("gh:ImJimmi/JIVE#main")
# (Alternative explicit form)
# CPMAddPackage(NAME JIVE GITHUB_REPOSITORY ImJimmi/JIVE GIT_TAG main)

# Tracktion-bundled JUCE
juce_add_modules(${TRACKTION_ENGINE_PATH}/modules/tracktion_graph)
juce_add_modules(${TRACKTION_ENGINE_PATH}/modules/tracktion_engine)

# ---- BinaryData (define before UI so UI sees <BinaryData.h>)
juce_add_binary_data(GrooveKitResources
        SOURCES
        resources/jive/drum_sampler.layout.xml
        resources/jive/drum_sampler.styles.json
)

# ---- Your libraries
add_subdirectory(src/AppEngine)
add_subdirectory(src/MIDIEngine)
add_subdirectory(src/AudioEngine)
add_subdirectory(src/UI)           # <-- now comes after GrooveKitResources
add_subdirectory(src/DatabaseManager)

# ---- App target (defines GrooveKit)
add_subdirectory(apps/GrooveKit)

# ---- Link once (cleaned)
target_link_libraries(GrooveKit
        PRIVATE
        GrooveKitResources
        jive::jive_layouts
        jive::jive_style_sheets
)

target_compile_definitions(GrooveKit PRIVATE
        JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS=1
)


target_link_libraries(GrooveKit
        PRIVATE
        jive::jive_layouts
        jive::jive_style_sheets
)
