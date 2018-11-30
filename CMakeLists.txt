cmake_minimum_required(VERSION 3.10)
project(mySqlBenchmark)

set(CMAKE_CXX_STANDARD 17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
endif ()

add_executable(mySqlBenchmark main.cpp)

if ( WIN32 )
    set(MYSQL_DEFAULT_DIR "C:/Program Files/MySQL/MySQL Connector C 6.1")
    set(MYSQL_CLIENT_NAME "libmysql")
    set(MYSQL_INCLUDE_DIR ${MYSQL_DEFAULT_DIR})
else ()
    set(MYSQL_DEFAULT_DIR "/usr")
    set(MYSQL_CLIENT_NAME "libmysqlclient.so")
    set(MYSQL_INCLUDE_DIR "/usr/include/mysql/")
endif ()

set(MYSQL_DIR ${MYSQL_DEFAULT_DIR} CACHE PATH "The path to the MySQL C API library")

message(STATUS "looking for mysql library in: " ${MYSQL_DIR})
find_library(mysql NAMES ${MYSQL_CLIENT_NAME} PATHS ${MYSQL_DIR}/lib)
add_library(mysql IMPORTED SHARED)
target_include_directories(mysql INTERFACE ${MYSQL_INCLUDE_DIR})
message(STATUS "mysql library: " ${mysql})

target_link_libraries(mySqlBenchmark
        mysql
        )
