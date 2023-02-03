add_library(HarmoPitchGetter
  HarmoPitchGetter.cpp
  HarmoPitchHelper.cpp
)

target_link_libraries(HarmoPitchGetter
  PUBLIC
    juce::juce_core
    juce::juce_audio_basics
)

add_executable(HarmoPitchGetterTests
  HarmoPitchGetterTests.cpp
)

target_link_libraries(HarmoPitchGetterTests
  PRIVATE
    HarmoPitchGetter
    gmock
    gtest_main
)