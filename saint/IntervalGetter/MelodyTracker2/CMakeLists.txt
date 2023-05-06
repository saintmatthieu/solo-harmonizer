add_library(MelodyTracker2
  MelodyTracker2.cpp
)

add_executable(MelodyTracker2Tests
  MelodyTracker2Tests.cpp
)

target_link_libraries(MelodyTracker2Tests
  PRIVATE
  gtest_main
  gmock
  MelodyTracker2
)