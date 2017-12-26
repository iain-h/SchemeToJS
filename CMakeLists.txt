# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

# Project's name

project(scm2js)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.45.0 COMPONENTS regex) 

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)

file(GLOB_RECURSE INSTRUCTIONS_SRCS
         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)

# The following folder will be included
include_directories("${PROJECT_SOURCE_DIR}/src")

file(GLOB INC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_definitions( -std=c++11 )

include_directories(${Boost_INCLUDE_DIRS}) 

add_executable(scm2js ${SRC_LIST} ${INC_LIST})

target_link_libraries(scm2js ${Boost_LIBRARIES})

