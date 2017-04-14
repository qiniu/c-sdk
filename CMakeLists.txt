cmake_minimum_required (VERSION 3.3.1)
project (libqiniu)

set (BUILD_SHARED_LIBS ON CACHE BOOL "Set to ON to build shared or dll libraries (default: ON)")
set (BUILD_STATIC_LIBS OFF CACHE BOOL "Set to ON to build static libraries (default: OFF)")
set (BUILD_X86_PLATFORM OFF CACHE BOOL "Set to ON to build for x86 platform (default: OFF)")
set (FUNCTION_LEVEL_LINKING OFF CACHE BOOL "Set to ON to place every function in its own section to garbadge-collect unreferenced ones")

file (GLOB_RECURSE MY_SOURCE_FILES qiniu/*.c b64/*.c cJSON/*.c)
include_directories (qiniu b64 cJSON)

### Platform Settings

set (MY_COMPILE_DEFINITIONS "")
set (MY_COMPILE_FLAGS "")

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")

    ## Append platform-dependent source files
    file (GLOB_RECURSE MY_WINDOWS_SOURCE_FILES windows/*.c)
    list (APPEND MY_SOURCE_FILES ${MY_WINDOWS_SOURCE_FILES})
    include_directories (windows)

    set (LOCAL_PACKAGE_PATH "C:/packages" CACHE PATH "Set to the path where dependent libraries (openssl / curl, etc) reside")
    include_directories (${LOCAL_PACKAGE_PATH}/include SYSTEM)
    link_directories (${LOCAL_PACKAGE_PATH}/lib)

    if (DEFINED BUILD_SHARED_LIBS AND ${BUILD_SHARED_LIBS})
        list (APPEND MY_COMPILE_DEFINITIONS COMPILING_QINIU_LIBRARY_DLL)
    endif (DEFINED BUILD_SHARED_LIBS AND ${BUILD_SHARED_LIBS})

else ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")

    include_directories (/usr/include /usr/local/include SYSTEM)
    link_directories (/usr/lib /usr/local/lib)

    set (MY_LINKING_LIBRARIES curl crypto)

    list (APPEND MY_COMPILE_FLAGS -Wall)

    if (DEFINED FUNCTION_LEVEL_LINKING AND ${FUNCTION_LEVEL_LINKING})
        list (APPEND MY_COMPILE_FLAGS -ffunction-sections)
    endif (DEFINED FUNCTION_LEVEL_LINKING AND ${FUNCTION_LEVEL_LINKING})

endif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")

if (DEFINED BUILD_X86_PLATFORM AND ${BUILD_X86_PLATFORM})
    list (APPEND CMAKE_C_FLAGS "-m32")
endif (DEFINED BUILD_X86_PLATFORM AND ${BUILD_X86_PLATFORM})

### Target Settings

if (DEFINED BUILD_SHARED_LIBS AND ${BUILD_SHARED_LIBS})

    add_library (qiniu SHARED ${MY_SOURCE_FILES})
    
    if (NOT ("X${MY_LINKING_LIBRARIES}" STREQUAL "X"))
        target_link_libraries (qiniu ${MY_LINKING_LIBRARIES})
    endif (NOT ("X${MY_LINKING_LIBRARIES}" STREQUAL "X"))

endif (DEFINED BUILD_SHARED_LIBS AND ${BUILD_SHARED_LIBS})

if (DEFINED BUILD_STATIC_LIBS AND ${BUILD_STATIC_LIBS})

    add_library (qiniu_a STATIC ${MY_SOURCE_FILES})

endif (DEFINED BUILD_STATIC_LIBS AND ${BUILD_STATIC_LIBS})

if (NOT ("X${MY_COMPILE_FLAGS}" STREQUAL "X"))
    set_target_properties (qiniu PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
endif (NOT ("X${MY_COMPILE_FLAGS}" STREQUAL "X"))

if (NOT ("X${MY_COMPILE_DEFINITIONS}" STREQUAL "X"))
    set_target_properties (qiniu PROPERTIES COMPILE_DEFINITIONS ${MY_COMPILE_DEFINITIONS})
endif (NOT ("X${MY_COMPILE_DEFINITIONS}" STREQUAL "X"))

add_subdirectory (demo)
