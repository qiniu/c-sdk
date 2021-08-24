CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
SET(PROJECT_NAME qiniu_test)
PROJECT(${PROJECT_NAME})

SET(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__SOURCE_DIR__=\"\\\"${CMAKE_SOURCE_DIR}\\\"\"")

INCLUDE(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

INCLUDE_DIRECTORIES("..")
AUX_SOURCE_DIRECTORY(. DIR_SRCS_TEST)
MESSAGE(STATUS "Src file: ${DIR_SRCS_TEST}")

ENABLE_TESTING()

ADD_EXECUTABLE(qiniu_test ${DIR_SRCS_TEST})
TARGET_LINK_LIBRARIES(qiniu_test PRIVATE gtest_main PUBLIC qiniu_static PUBLIC ${CURL_LIBRARIES} PUBLIC ${OPENSSL_LIBRARIES})

INCLUDE(GoogleTest)
GTEST_DISCOVER_TESTS(qiniu_test)
