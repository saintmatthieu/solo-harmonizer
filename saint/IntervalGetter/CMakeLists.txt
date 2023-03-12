add_library(IntervalGetter
  IntervalGetter.cpp
  IntervalHelper.cpp
)

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
  IntervalGetterTests.cpp
  IntervalHelperTests.cpp
)

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