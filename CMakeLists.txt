cmake_minimum_required(VERSION 3.10)

project(CommonConnectionPool)

set(CMAKE_BUILD_TYPE "Debug")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(/usr/include/mysql ${PROJECT_SOURCE_DIR}/src/include)

set(SRC_LIST
    src/Connection.cc
    src/ConnectionPool.cc
    src/main.cc)

add_executable(test ${SRC_LIST})

target_link_libraries(test mysqlclient pthread)