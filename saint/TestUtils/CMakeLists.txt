add_library(TestUtils)

target_sources(TestUtils
  PUBLIC
    testUtils.cpp
)

target_link_libraries(TestUtils
  PUBLIC
    gtest_main
    gmock
    juce::juce_audio_utils
)

target_include_directories(TestUtils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
