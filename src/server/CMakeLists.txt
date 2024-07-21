cmake_minimum_required(VERSION 3.0)
project(chatserver)

#设置源文件目录
file(GLOB_RECURSE SRC_LIST ./*.cpp)
#file(GLOB SRC_LIST ./chatservice.cpp ./chatserver.cpp ./main.cpp )

#生成可执行文件
add_executable(server ${SRC_LIST})

#连接静态库或动态库
target_link_libraries(server muduo_net muduo_base mysqlclient hiredis pthread)