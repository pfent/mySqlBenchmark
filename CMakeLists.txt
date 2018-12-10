cmake_minimum_required(VERSION 3.10)
project(mySqlBenchmark)

set(CMAKE_CXX_STANDARD 17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
endif ()

add_executable(mySqlBenchmark main.cpp)

if ( WIN32 )
    set(MYSQL_DEFAULT_DIR "C:/Program Files/MySQL/MySQL Connector C 6.1")
    set(MYSQL_CLIENT_NAME "/lib/libmysql.dll")
    set(MYSQL_INCLUDE_DIR "${MYSQL_DEFAULT_DIR}/include")
else ()
    set(MYSQL_DEFAULT_DIR "/usr")
    set(MYSQL_CLIENT_NAME "/lib/libmysqlclient.so")
    set(MYSQL_INCLUDE_DIR "/usr/include/mysql/")
endif ()

set(MYSQL_DIR ${MYSQL_DEFAULT_DIR} CACHE PATH "The path to the MySQL C API library")

message(STATUS "looking for mysql library in: " ${MYSQL_DIR})
add_library(mysql SHARED IMPORTED SHARED)
target_include_directories(mysql INTERFACE ${MYSQL_INCLUDE_DIR})
if(WIN32)
	set_target_properties(mysql PROPERTIES IMPORTED_IMPLIB "${MYSQL_DEFAULT_DIR}${MYSQL_CLIENT_NAME}")
else()
	set_target_properties(mysql PROPERTIES IMPORTED_LOCATION "${MYSQL_DEFAULT_DIR}${MYSQL_CLIENT_NAME}")
endif()
message(STATUS "mysql library: ${MYSQL_DEFAULT_DIR}${MYSQL_CLIENT_NAME}")

target_link_libraries(mySqlBenchmark
		mysql
        )
