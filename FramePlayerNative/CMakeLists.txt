cmake_minimum_required(VERSION 3.18.1)

project(FramePlayerNative LANGUAGES C CXX)
set(PRODUCT ${PROJECT_NAME})

set(${PRODUCT}_ROOT ${PROJECT_SOURCE_DIR})

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Installation Directory")

include(${${PRODUCT}_ROOT}/cmake/common.cmake)
message("CMake Created: RootDir is" ${${PRODUCT}_ROOT})

option(BUILD_FPN_LIBRARY "Build fpn as a library" OFF)
option(BUILD_FPN_EXTRA "Build fpn extra capabilities" OFF)
option(BUILD_FPN_ENABLE_OPENGL_API "Build fpn opengl capabilities" ON)

add_subdirectory(source)