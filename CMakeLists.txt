cmake_minimum_required(VERSION 2.6)

project(libvsqx)
project(test1)

include(FindPkgConfig)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

message(${CMAKE_MODULE_PATH})

#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++11")#enable incase we need it
INCLUDE_DIRECTORIES(./)

find_package(TinyXML2 REQUIRED)

add_library(libvsqx SHARED libvsqx.cpp)
include_directories(${TINYXML2_INCLUDE_DIRS})
target_link_libraries(libvsqx ${TINYXML2_LIBRARIES})

add_executable(test1 test1.cpp)
target_link_libraries(test1 libvsqx)
