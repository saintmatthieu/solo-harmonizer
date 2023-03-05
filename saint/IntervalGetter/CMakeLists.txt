add_library(IntervalGetter
  IntervalGetter.cpp
  IntervalHelper.cpp
  IntervalTypes.cpp
)

target_include_directories(IntervalGetter
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(IntervalGetter
  PUBLIC
    TestUtils
)

add_executable(IntervalGetterTests
  IntervalGetterTests.cpp
  IntervalHelperTests.cpp
)

target_link_libraries(IntervalGetterTests
  PRIVATE
    IntervalGetter
    ${JuceLibDeps_IntervalGetter}
    gmock
    gtest_main
)