cmake_minimum_required(VERSION 3.10)
project(mySqlBenchmark)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(ext/mysql-connector-cpp)
add_executable(mySqlBenchmark main.cpp)

target_include_directories(mySqlBenchmark
        PRIVATE .
        PRIVATE ext/mysql-connector-cpp/include
        )

target_link_libraries(mySqlBenchmark
        connector
        )
