CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

CMAKE_POLICY(SET CMP0048 NEW)
CMAKE_POLICY(SET CMP0074 NEW)

SET(PROJECT_NAME examples)
PROJECT(${PROJECT_NAME} LANGUAGES C)

set(CMAKE_C_STANDARD 99)

IF(WIN32)
    ADD_COMPILE_OPTIONS("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    ADD_COMPILE_OPTIONS("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
ENDIF()

INCLUDE_DIRECTORIES(".." ".")

FILE(GLOB example_files "*.c")
FOREACH(file ${example_files})
    GET_FILENAME_COMPONENT(example_file "${file}" NAME)
    GET_FILENAME_COMPONENT(program_name "${file}" NAME_WE)
    ADD_EXECUTABLE(${program_name} "${example_file}")
    IF(WIN32)
        TARGET_LINK_LIBRARIES(${program_name} PUBLIC qiniu_static PUBLIC ${CURL_LIBRARIES} PUBLIC ${OPENSSL_LIBRARIES})
    ELSE()
        TARGET_LINK_LIBRARIES(${program_name} PUBLIC qiniu_static PUBLIC ${CURL_LIBRARIES} PUBLIC ${OPENSSL_LIBRARIES} PUBLIC m)
    ENDIF()
ENDFOREACH()
