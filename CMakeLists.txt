cmake_minimum_required(VERSION 3.10)

project(libwz)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# bianry output 

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_BINARY_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_BUILD_TYPE "Debug")

# zlib dependency
find_package(ZLIB REQUIRED)
include_directories(ZLIB_INCLUDE_DIRS)

# boost library
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.71 COMPONENTS thread system log locale REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# openssl
find_package(OpenSSL REQUIRED)
include_directories(${BoringSSL_INCLUDE_DIR})

# gtest
enable_testing()
find_package(GTest REQUIRED)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test")