cmake_minimum_required(VERSION 3.10)

add_library(wz STATIC
    wzfile.cc
    wzreader.cc
    utils.cc
    wznode.cc
    wzkey.cc
)
target_link_libraries(
    wz
    ${ZLIB_LIBRARIES}
)