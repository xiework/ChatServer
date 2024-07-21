cmake_minimum_required(VERSION 3.0)
project(chatserver)

#设置源文件目录
file(GLOB_RECURSE SRC_LIST ./*.cpp)

#生成可执行文件
add_executable(client ${SRC_LIST})

#连接静态库或动态库
target_link_libraries(client pthread)