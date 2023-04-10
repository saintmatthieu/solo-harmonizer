add_library(IntervalGetter
  MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.cpp
  MelodyTracker/TimingEstimator/DefaultTimingEstimator.cpp
  MelodyTracker/DefaultMelodyTracker.cpp
  DefaultIntervalGetter.cpp
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
  MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizerTests.cpp
  MelodyTracker/DefaultMelodyTrackerTests.cpp
  DefaultIntervalGetterTests.cpp
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