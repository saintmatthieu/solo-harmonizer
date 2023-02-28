add_library(PitchDetector)

target_sources(PitchDetector
  PUBLIC
    PitchDetector.cpp
)

target_link_libraries(PitchDetector
  PUBLIC
    PFFFT
)

target_include_directories(PitchDetector
  PUBLIC
    ${CMAKE_SOURCE_DIR}/_thirdParty
    ${CMAKE_SOURCE_DIR}/pffft
    ${CMAKE_SOURCE_DIR}/Ring-Buffer
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(PitchDetectorTests
  PitchDetectorTests.cpp
)

target_link_libraries(PitchDetectorTests
  PRIVATE
    TestUtils
    PitchDetector
)
