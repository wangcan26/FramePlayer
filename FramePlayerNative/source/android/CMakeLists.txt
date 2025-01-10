get_filename_component(target ${CMAKE_CURRENT_LIST_DIR} NAME)

set(target fpn_android)

add_library(${target} STATIC "")

file(GLOB_RECURSE srcs *.h *.c *.cpp)

target_sources(${target} PRIVATE ${srcs})
set_target_properties(${target} PROPERTIES CXX_STANDARD 17)
#include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${target} PUBLIC android fpn_log)