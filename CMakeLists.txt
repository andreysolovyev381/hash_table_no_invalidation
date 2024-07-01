cmake_minimum_required(VERSION 3.19)
set (PROJECT_NAME "Open addressing Hash table")
project(${PROJECT_NAME}
        VERSION 0.1
        DESCRIPTION "Hash table with Open addressing and no invalidation whatsoever, using C++20"
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(./include)
add_subdirectory(./tests)
add_subdirectory(./benchmark)