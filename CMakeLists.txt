#设置CMAKE最低版本
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#设置项目名称
SET(PROJECT_NAME qiniu)
#建立项目
PROJECT(${PROJECT_NAME})

#添加C++11支持
#include(CheckCXXCompilerFlag) 
#CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)  
#CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)  
#if(COMPILER_SUPPORTS_CXX11)  
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")  
#elseif(COMPILER_SUPPORTS_CXX0X)  
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")  
#else()  
#    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")  
#endif()
#添加C99支持
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g")
#添加GNU99支持 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -g")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
include_directories(/usr/local/opt/openssl/include)
link_directories(/usr/local/opt/openssl/lib)
endif()

AUX_SOURCE_DIRECTORY(b64 DIR_SRCS_B64)
AUX_SOURCE_DIRECTORY(cJSON DIR_SRCS_CJSON)
AUX_SOURCE_DIRECTORY(qiniu DIR_SRCS_QINIU)
MESSAGE(STATUS "Src file: ${DIR_SRCS_B64} ${DIR_SRCS_CJSON} ${DIR_SRCS_QINIU}")
#编译可执行程序
#ADD_EXECUTABLE(${PROJECT_NAME} ${DIR_SRCS})
#如果要生成静态链接库
#ADD_LIBRARY(${PROJECT_NAME} STATIC ${DIR_SRCS_B64} ${DIR_SRCS_CJSON} ${DIR_SRCS_QINIU})
#如果要生成动态链接库
ADD_LIBRARY(${PROJECT_NAME} SHARED ${DIR_SRCS_B64} ${DIR_SRCS_CJSON} ${DIR_SRCS_QINIU})

#添加链接库
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  -lcurl -lcrypto -lssl -lm)
