cmake_minimum_required(VERSION 3.10)

add_library(wz STATIC
    wzfile.cc
    wzreader.cc
    utils.cc
    wznode.cc
    wzkey.cc
    nx/wz2nx_serializer.cc
)
target_link_libraries(
    wz
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    OpenSSL::Crypto
)

add_executable(wz2nx
    nx/wz2nx_main.cc
)
target_link_libraries(
    wz2nx
    wz
)