cmake_minimum_required(VERSION 3.6.0)

set(SOURCE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -fvisibility=hidden -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections -fvisibility=hidden -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti")

include_directories(PUBLIC ${SOURCE_ROOT_DIR})

if(BUILD_FPN_ENABLE_OPENGL_API)
    add_definitions(-DFPN_USE_OPENGL_API=1)
endif()

add_subdirectory(log)
if(TARGET_OS_ANDROID)
    add_subdirectory(android)
endif()
if(BUILD_FPN_EXTRA)
    add_definitions(-DFPN_USE_EXTRA_RENDER=1)
    add_subdirectory(extra/render)
    add_definitions(-DFPN_USE_EXTRA_PRODUCER=1)
    add_subdirectory(extra/producer)
endif()

add_subdirectory(main)


