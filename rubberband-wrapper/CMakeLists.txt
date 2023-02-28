add_library(rubberband-wrapper
    ${CMAKE_SOURCE_DIR}/rubberband/single/RubberBandSingle.cpp
)

target_include_directories(rubberband-wrapper
  PUBLIC
  ${CMAKE_SOURCE_DIR}/rubberband
)

target_compile_definitions(rubberband-wrapper PRIVATE NOMINMAX)
