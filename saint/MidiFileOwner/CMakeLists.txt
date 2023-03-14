add_library(MidiFileOwner
  DefaultMidiFileOwner.cpp
  JuceMidiFileUtils.cpp
  PositionGetter.cpp
)

target_compile_options(MidiFileOwner PRIVATE ${SAINT_ANNOYING_WARNINGS})

target_include_directories(MidiFileOwner
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MidiFileOwner
  PUBLIC
    IntervalGetter
    Utils
)

add_executable(PositionGetterTests
  PositionGetterTests.cpp
)

target_compile_options(PositionGetterTests PRIVATE ${SAINT_ANNOYING_WARNINGS})

target_link_libraries(PositionGetterTests
  PRIVATE
    MidiFileOwner
    ${JuceLibDeps_MidiFileOwner}
    juce::juce_audio_formats
    juce::juce_graphics # else I get error: undefined symbol: public: __cdecl juce::Colour::Colour(unsigned int) - but why ??
    gmock
    gtest_main
)