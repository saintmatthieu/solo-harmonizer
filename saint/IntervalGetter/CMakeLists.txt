add_library(IntervalGetter
  IntervalGetter.cpp
  IntervalHelper.cpp
  IntervalTypes.cpp
)

target_include_directories(IntervalGetter
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(IntervalGetterTests
  IntervalGetterTests.cpp
  IntervalHelperTests.cpp
)

target_link_libraries(IntervalGetterTests
  PRIVATE
    IntervalGetter
    gmock
    gtest_main
)