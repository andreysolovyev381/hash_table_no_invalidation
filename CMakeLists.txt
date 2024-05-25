cmake_minimum_required(VERSION 3.25)
set (PROJECT_NAME "Hash table, Chain addressing, no invalidation whatsoever")
project(${PROJECT_NAME}
        VERSION 0.1
        DESCRIPTION "Hash table using C++20"
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(./tests)
add_subdirectory(./benchmark)