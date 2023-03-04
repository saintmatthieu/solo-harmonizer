add_library(Utils)

target_sources(Utils
  PUBLIC
    Utils.cpp
)

target_include_directories(Utils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
