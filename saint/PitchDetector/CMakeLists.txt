add_library(PitchDetector)

target_sources(PitchDetector
  PUBLIC
    PitchDetectorImpl.cpp
)

target_link_libraries(PitchDetector
  PUBLIC
    PFFFT
    TestUtils
)

target_include_directories(PitchDetector
  PUBLIC
    ${CMAKE_SOURCE_DIR}/_thirdParty
    ${CMAKE_SOURCE_DIR}/Ring-Buffer
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(PitchDetector SYSTEM
  PUBLIC
    ${CMAKE_SOURCE_DIR}/pffft
)

add_executable(PitchDetectorImplTests
  PitchDetectorImplTests.cpp
)

target_link_libraries(PitchDetectorImplTests
  PRIVATE
    PitchDetector
    ${JuceLibDeps_PitchDetector}
    gtest_main
    gmock
)
