get_filename_component(target ${CMAKE_CURRENT_LIST_DIR} NAME)

set(target fpn_extra_producer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(${target} STATIC "")

file(GLOB_RECURSE srcs *.h *.c *.cpp)

target_sources(${target} PRIVATE ${srcs})
set_target_properties(${target} PROPERTIES CXX_STANDARD 17)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${target} PUBLIC android fpn_log)