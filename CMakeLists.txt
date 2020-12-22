cmake_minimum_required(VERSION 3.10)

project(libwz)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# bianry output 

set(CMAKEols/bionic,now 65.1-1+ubuntu18.04.1+deb.s_LIBRARY_OUTPUT_DIRECTORY  "${PROJECT_BINARY_DIR}/bin")
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

# libicu
find_package(ICU REQUIRED COMPONENTS uc dt in io)
include_directories(${ICU_INCLUDE_DIRS})
link_directories(${ICU_LIBRARY_DIRS})

# liblz4-dev
find_package(LZ4 REQUIRED liblz4)
include_directories(${LZ4_INCLUDE_DIR})
link_directories(${LZ4_LIBRARY})


# gtest
enable_testing()
find_package(GTest REQUIRED)

# squish
find_package(Squish 3.0)

 

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test")