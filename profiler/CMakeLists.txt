set(EXECUTABLE_NAME hash_table_profiler_helper)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
            -g
            -O0
            -march=native
            -fconcepts-diagnostics-depth=3
    )
else()
    add_compile_options(
            -g
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
        pthread
)
