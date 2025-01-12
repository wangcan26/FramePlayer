get_filename_component(target ${CMAKE_CURRENT_LIST_DIR} NAME)

set(target fpn_mono)
add_library(${target} SHARED "")

file(GLOB_RECURSE srcs *.h *.c *.cpp)
target_sources(${target} PRIVATE ${srcs})
set_target_properties(${target} PROPERTIES CXX_STANDARD 17)
#include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR})

set(LIBS EGL GLESv3)
if(BUILD_FPN_EXTRA)
    set(LIBS ${LIBS} fpn_extra_render)
    set(LIBS ${LIBS} fpn_extra_producer)
endif()

target_link_libraries(${target} PUBLIC fpn_android fpn_log ${LIBS})