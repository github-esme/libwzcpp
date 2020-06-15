cmake_minimum_required(VERSION 3.10)

add_executable(test_wz
    test_reader.cc
)

target_link_libraries(
        test_wz 
        wz 
        GTest::GTest 
        GTest::Main 
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        )

add_test(NAME test_wz COMMAND test)