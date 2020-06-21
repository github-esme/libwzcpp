cmake_minimum_required(VERSION 3.10)

add_executable(test_wz
    test_reader.cc
    test_wzkey.cc
)

target_link_libraries(
        test_wz 
        wz 
        GTest::GTest 
        GTest::Main 
        )

add_test(NAME test_wz COMMAND test)