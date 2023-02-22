add_library(OnsetDetector)

target_sources(OnsetDetector
  PUBLIC
    OnsetDetector.cpp
)

target_link_libraries(OnsetDetector
  PUBLIC
    OnsetsDS
    PFFFT
)

target_include_directories(OnsetDetector
  PUBLIC
    ${CMAKE_SOURCE_DIR}/_thirdParty
    ${CMAKE_SOURCE_DIR}/pffft
    ${CMAKE_SOURCE_DIR}/Ring-Buffer
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(OnsetDetectorTests
  OnsetDetectorTests.cpp
)

target_link_libraries(OnsetDetectorTests
  PRIVATE
    TestUtils
    OnsetDetector
)
