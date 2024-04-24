add_executable(testQueues
    testQueues.cpp)

target_link_libraries(testQueues
    gtest_main)

include(GoogleTest)
gtest_discover_tests(testQueues)
