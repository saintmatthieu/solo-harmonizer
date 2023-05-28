add_library(MelodyRecognizer3
  MelodyRecognizer3.cpp
)

add_executable(MelodyRecognizer3Tests
  MelodyRecognizer3Tests.cpp
)

target_link_libraries(MelodyRecognizer3Tests
  PRIVATE
  gtest_main
  gmock
  MelodyRecognizer3
)