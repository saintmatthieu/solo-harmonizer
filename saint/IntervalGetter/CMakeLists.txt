add_library(IntervalGetter
  DefaultIntervalGetter.cpp
  DefaultMelodyFollower.cpp
  DefaultPerformanceTimeWarper.cpp
  DefaultPitchMapper.cpp
  DefaultPitchMapperHelper.cpp
  KeyRecognizer.cpp
  KeyRecognizerHelper.cpp
  KrumhanslSchmucklerAlgorithm.cpp
  StochasticIntervalGetter.cpp
  IntervalGetter.cpp
  IntervalHelper.cpp
)

target_compile_options(IntervalGetter PRIVATE ${SAINT_ANNOYING_WARNINGS})

target_include_directories(IntervalGetter
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(IntervalGetter
  PUBLIC
    TestUtils
    Utils
)

add_executable(IntervalGetterTests
  DefaultIntervalGetterTests.cpp
  DefaultMelodyFollowerTests.cpp
  DefaultPitchMapperHelperTests.cpp
  IntervalHelperTests.cpp
  KeyRecognizerHelperTests.cpp
  KrumhanslSchmucklerAlgorithmTests.cpp
)

target_compile_options(IntervalGetterTests PRIVATE ${SAINT_ANNOYING_WARNINGS})

target_include_directories(IntervalGetterTests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/_thirdParty/asiosdk/common # Needed by JUCE
)

target_link_libraries(IntervalGetterTests
  PRIVATE
    IntervalGetter
    ${JuceLibDeps_IntervalGetter}
    gmock
    gtest_main
)