add_library(DavidCNAntonia)

target_sources(DavidCNAntonia
    PRIVATE
        PitchShifter.cpp
        RingBuffer.cpp)

target_compile_definitions(DavidCNAntonia PUBLIC JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED)

target_link_libraries(DavidCNAntonia
    PUBLIC
        rubberband-wrapper)

target_include_directories(DavidCNAntonia
    PRIVATE
        ${CMAKE_SOURCE_DIR}/JUCE/modules
)
