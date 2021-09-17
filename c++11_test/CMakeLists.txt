cmake_minimum_required(VERSION 3.19)
project(c__11_test)

set(CMAKE_CXX_STANDARD 11)

if(WIN32)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_subdirectory(./sqlite3-cmake-master)

include_directories(./sqlite3-cmake-master/src)

file(GLOB_RECURSE FILES test_*.cpp)
foreach(FILE ${FILES})
    string(REGEX REPLACE ".+/(.+)\\..*$" "\\1" TARGET ${FILE})
    add_executable(${TARGET} ${FILE})
    target_link_libraries(${TARGET} pthread)
endforeach()

target_link_libraries(test_sqlite_cpp sqlite3 pthread dl)
