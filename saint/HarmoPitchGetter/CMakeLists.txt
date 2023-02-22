add_library(HarmoPitchGetter
  HarmoPitchGetter.cpp
  HarmoPitchHelper.cpp
  HarmoPitchTypes.cpp
)

target_link_libraries(HarmoPitchGetter
  PUBLIC
    juce::juce_core
    juce::juce_audio_basics
)

target_include_directories(HarmoPitchGetter
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(HarmoPitchGetterTests
  HarmoPitchGetterTests.cpp
  HarmoPitchHelperTests.cpp
)

target_link_libraries(HarmoPitchGetterTests
  PRIVATE
    HarmoPitchGetter
    gmock
    gtest_main
)