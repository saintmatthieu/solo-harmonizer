add_library(MelodyRecognizer2
  MelodyRecognizer2.cpp
)

add_executable(MelodyRecognizer2Tests
  MelodyRecognizer2Tests.cpp
)

target_link_libraries(MelodyRecognizer2Tests
  PRIVATE
  gtest_main
  gmock
  MelodyRecognizer2
)