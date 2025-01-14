get_filename_component(target ${CMAKE_CURRENT_LIST_DIR} NAME)

set(LIBS EGL GLESv3)
if(BUILD_FPN_EXTRA)
    set(LIBS ${LIBS} fpn_extra_render)
    set(LIBS ${LIBS} fpn_extra_producer)
endif()

if(BUILD_FPN_LIBRARY)
set(target fpn_main)
add_library(${target} STATIC "")
set_target_properties(${target} PROPERTIES CXX_STANDARD 17)

file(GLOB_RECURSE srcs *.h *.c *.cpp)
target_sources(${target} PRIVATE ${srcs})
target_link_libraries(${target} PUBLIC android fpn_log ${LIBS})

file(GLOB_RECURSE headers *.h)
message("main collect headers: ${headers}")
set_target_properties(${target} PROPERTIES PUBLIC_HEADER "${headers}")

INSTALL(TARGETS ${target} 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/main
)

else()
set(target fpn_mono)
add_library(${target} SHARED "")

file(GLOB_RECURSE srcs *.h *.c *.cpp)
target_sources(${target} PRIVATE ${srcs})
set_target_properties(${target} PROPERTIES CXX_STANDARD 17)
#include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${target} PUBLIC fpn_android fpn_log ${LIBS})
endif()
