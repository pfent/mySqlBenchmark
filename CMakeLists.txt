cmake_minimum_required(VERSION 3.10)
project(mySqlBenchmark)

set(CMAKE_CXX_STANDARD 17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
endif ()

add_executable(mySqlBenchmark main.cpp)

set(MYSQL_DIR "C:/Program Files/MySQL/MySQL Connector C 6.1"    CACHE PATH "The path to the MySQL C API library")
include_directories(${MYSQL_DIR}/include)

find_library(mysql NAMES libmysql PATHS ${MYSQL_DIR}/lib)
message(STATUS "mysql library: " ${mysql})

target_link_libraries(mySqlBenchmark
        ${mysql}
        )
