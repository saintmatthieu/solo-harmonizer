add_library(TestUtils)

target_sources(TestUtils
  PUBLIC
    testUtils.cpp
    PitchDetectorImplDebugCb.cpp
)

target_link_libraries(TestUtils
PUBLIC
    juce::juce_audio_utils
)

target_include_directories(TestUtils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
