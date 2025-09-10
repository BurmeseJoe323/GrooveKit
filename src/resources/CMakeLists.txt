file(GLOB_RECURSE DRUM_WAVS
        ${CMAKE_SOURCE_DIR}/src/resources/Samples/*.[Ww][Aa][Vv]
)

list(LENGTH DRUM_WAVS NUM_DRUM_WAVS)
message(STATUS "GrooveKit: found ${NUM_DRUM_WAVS} default WAV(s) to embed")
foreach(w ${DRUM_WAVS})
    message(STATUS "  WAV: ${w}")
endforeach()

target_sources(GrooveKitResources PRIVATE ${DRUM_WAVS})