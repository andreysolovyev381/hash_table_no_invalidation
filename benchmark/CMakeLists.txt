include(FetchContent)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)
# For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_USE_BUNDLED_GTEST OFF)

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.3
)
FetchContent_MakeAvailable(benchmark)

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
