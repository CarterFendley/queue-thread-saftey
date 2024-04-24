cmake_minimum_required(VERSION 3.16)
project(queue-thread-saftey CXX)

set(USE_CXX17 ON)

if (DEBUG_UNOPT)
    set(OPTIMIZATION_FLAG "-O0")
else()
    set(OPTIMIZATION_FLAG "-O3")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-vla -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${OPTIMIZATION_FLAG} -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(thirdparty/googletest)
enable_testing()

add_subdirectory(src)
