if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(WASM TRUE)
    set(TARGET_OS "wasm")
    add_definitions(-DTARGET_OS_WASM=1)
    set(TARGET_OS_WASM TRUE)
    set(LINUX FALSE)
    set(WINDOWS FALSE)
    set(MAC FALSE)
else()
    set(TARGET_OS_WASM FALSE)

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(LINUX TRUE)
        set(TARGET_OS "linux")
        set(TARGET_OS_LINUX TRUE)
        add_definitions(-DTARGET_OS_LINUX=1)
    else()
        set(LINUX FALSE)
    endif()

    if(MSVC12 OR MSVC14 OR WIN32)
        set(WINDOWS TRUE)
        set(TARGET_OS "windows")
        set(TARGET_OS_WINDOWS TRUE) # render_core
        add_definitions(-DTARGET_OS_WINDOWS=1) # render_core
    else()
        set(WINDOWS FALSE)
    endif()

    if(IOS)
        set(TARGET_OS_IPHONE TRUE)
        set(TARGET_OS "ios")
        add_definitions(-DTARGET_OS_IPHONE=1)
    endif()

    if(APPLE AND NOT IOS)
        set(MAC TRUE)
        set(TARGET_OS "mac")
        add_definitions(-DTARGET_OS_MAC=1)
    else()
        set(MAC FALSE)
    endif()

    if(IOS)
        set(TARGET_OS_IPHONE TRUE)
        set(TARGET_OS "ios")
    endif()

    if(ANDROID)
        set(TARGET_OS_ANDROID TRUE)
        set(TARGET_OS "android")
        set(TARGET_ARCH_ABI ${ANDROID_ABI})
        add_definitions(-DTARGET_OS_ANDROID=1)
    endif()

    if(MAC)
        set(TARGET_OS_MAC TRUE)
    endif()
endif()

function(active_module module_name)
    if ($ENV{CI_BUILD})
        if (${module_name})
            message(STATUS "-->>CI_BUILD: active module: ${module_name}")
            set(${module_name} 1 PARENT_SCOPE)
            add_definitions(-D${module_name}=1)
            set(G_ALL_ACTIVE_MODULES ${G_ALL_ACTIVE_MODULES} ${module_name} PARENT_SCOPE)
        endif ()
    else ()
        message(STATUS "-->>active module: ${module_name}")
        set(${module_name} 1 PARENT_SCOPE)
        add_definitions(-D${module_name}=1)
        set(G_ALL_ACTIVE_MODULES ${G_ALL_ACTIVE_MODULES} ${module_name} PARENT_SCOPE)
    endif ()
endfunction()

macro(RECURSIVE_FILE_SOURCE src_path_list return_src_source)
    set(SOURCES_PARRTEN ${src_path_list}/*.cpp ${src_path_list}/*.c ${src_path_list}/*.h ${src_path_list}/*.hpp ${src_path_list}/*.cc ${src_path_list}/*.proto)
    if(IOS OR MAC)
        list(APPEND SOURCES_PARRTEN ${src_path_list}/*.m ${src_path_list}/*.mm)
    endif()
    file(GLOB_RECURSE all_files ${SOURCES_PARRTEN})
    list(APPEND ${return_src_source} ${all_files})
endmacro()

macro(RECURSIVE_FILE_HEADERS src_path_list return_src_headers)
    file(GLOB_RECURSE all_files ${src_path_list}/*.h ${src_path_list}/*.hpp)
    set(${return_src_headers} ${all_files})
endmacro()

function(ADD_FEATURE_SOURCE src_path_list return_src_source)
    RECURSIVE_FILE_SOURCE(${src_path_list} TEMP)
    set(${return_src_source} ${${return_src_source}} ${TEMP} PARENT_SCOPE)
endfunction()

macro(RECURSIVE_HEADER_DIR root_path return_headers_dir)
    file(GLOB_RECURSE all_files ${root_path}/*.h ${root_path}/*.hpp)
    set(return_headers_dir "")
    foreach(file_path ${all_files})
        get_filename_component(dir_path ${file_path} PATH)
        list(APPEND ${return_headers_dir} ${dir_path})
    endforeach()
    list(REMOVE_DUPLICATES return_headers_dir)
endmacro()

macro(add_android_static_libs LIB_DIR)
    foreach(LIB_NAME ${ARGN})
      add_library(${LIB_NAME}
            STATIC
            IMPORTED)
      
      set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION
      ${LIB_DIR}/lib${LIB_NAME}.a)
    endforeach(LIB_NAME ${ARGN})
endmacro(add_android_static_libs)


macro(add_android_shared_libs LIB_DIR)
   foreach(LIB_NAME ${ARGN})
        add_library(${LIB_NAME}
            SHARED
            IMPORTED)
      
        set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION
        ${LIB_DIR}/lib${LIB_NAME}.so)
    endforeach(LIB_NAME ${ARGN})
endmacro(add_android_shared_libs)