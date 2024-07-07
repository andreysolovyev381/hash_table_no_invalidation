set(EXECUTABLE_NAME hash_table_benchmark)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
            -g
            -O0
            -march=native
            -fconcepts-diagnostics-depth=3
    )
else()
    add_compile_options(
            -O3
            -march=native
    )
endif()
add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

find_package(benchmark REQUIRED)
if(benchmark_FOUND)
    message(STATUS "BENCHMARK FOUND, including and linking...")
    include_directories(${benchmark_INCLUDE_DIRS})
else()
    message(STATUS "NOT FOUND BENCHMARK")
endif()

add_executable(${EXECUTABLE_NAME}
        ./main.cpp
)
target_link_libraries(${EXECUTABLE_NAME}
        benchmark::benchmark
        pthread
)
target_compile_options(${EXECUTABLE_NAME}
        PRIVATE
        -Wno-uninitialized
)
