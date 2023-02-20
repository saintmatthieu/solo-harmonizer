add_library(OnsetDetector)

target_sources(OnsetDetector
  PUBLIC
    OnsetDetector.cpp
)

target_link_libraries(OnsetDetector
  PUBLIC
    OnsetsDS
    juce::juce_dsp
)

target_include_directories(OnsetDetector
  PUBLIC
    ${CMAKE_SOURCE_DIR}/_thirdParty
)
