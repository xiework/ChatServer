cmake_minimum_required (VERSION 3.0) #要求cmake最低的版本
project(demo) #项目名称
#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 设置指定的C++编译器版本是必须的，如果不设置，或者为OFF，则指定版本不可用时，会使用上一版本。
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定为C++11 版本
set(CMAKE_CXX_STANDARD 11)

#设置生成的可执行文件的目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

#设置源文件目录
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

#生成可执行文件
add_executable(muduo_server ${SRC_LIST})

#连接静态库或动态库
target_link_libraries(muduo_server muduo_net muduo_base pthread)