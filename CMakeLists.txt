cmake_minimum_required(VERSION 2.8)

add_definitions(-Wall)
add_definitions(-Wno-deprecated)
add_definitions(-O2)
add_definitions(-g)

#头文件引用地址中
#curl的地址请根据开发机器上的实际地址修改
include_directories(./
	./b64
        ./cJSON
        ./qiniu
	./curl
        )

aux_source_directory(./b64 SRCS1)
aux_source_directory(./cJSON SRCS2)
aux_source_directory(./qiniu SRCS3)
add_library(qiniu STATIC ${SRCS1} ${SRCS2} ${SRCS3})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
