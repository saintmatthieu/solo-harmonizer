add_library(TestUtils)

target_compile_options(TestUtils PRIVATE ${SAINT_ANNOYING_WARNINGS})

target_sources(TestUtils
  PUBLIC
    testUtils.cpp
    IntervalGetterDebugCb.cpp
    PerformanceTimeWarperDebugCb.cpp
    PitchDetectorDebugCb.cpp
    WavFileReader.cpp
    WavFileWriter.cpp
)

target_compile_definitions(TestUtils
  PUBLIC
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED
)

target_link_libraries(TestUtils
  PUBLIC
    Utils
)

target_include_directories(TestUtils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/JUCE/modules
)
