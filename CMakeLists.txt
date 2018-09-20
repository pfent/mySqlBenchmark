cmake_minimum_required(VERSION 3.10)
project(mySqlBenchmark)

set(CMAKE_CXX_STANDARD 17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
endif ()

add_executable(mySqlBenchmark main.cpp)

target_link_libraries(mySqlBenchmark
        mysqlclient
        )
