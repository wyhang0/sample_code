# windows使用cmake编译protobuf. 准备vs2019环境
# 1.新建build目录
# 2.在新建的目录下执行 cmake /path/to/protobuf/cmake/
# 3.使用vs2019编译protobuf库
# 4.把库拷贝到lib目录下
# 5.新建include/google/protobuf/目录，把/path/to/protobuf/src/google/protobuf/下的文件拷贝到include/google/protobuf/
cmake_minimum_required(VERSION 3.15)

project(protobuf_test)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
link_directories(lib)

add_executable(main main.cpp a.pb.cc)
target_link_libraries(main libprotobufd)

