cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DNOMINMAX)

project(SoloHarmonizer VERSION 0.0.1)

add_subdirectory(rubberband-wrapper)
add_subdirectory(LibPyin)
add_subdirectory(googletest)
add_subdirectory(JUCE)
add_subdirectory(spdlog)
add_subdirectory(saint)
