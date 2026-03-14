include(FetchContent)
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

set(TESTS_NAME hash_table_tests)

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

add_executable(${TESTS_NAME}
        ./hash_table_set.cpp
        ./hash_table_map.cpp
        ./pmr.cpp
        ./requirements.cpp
        ./main.cpp
)
target_compile_definitions(${TESTS_NAME} PUBLIC CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(${TESTS_NAME}
        GTest::gtest_main
        pthread
)
# target_compile_options(${TESTS_NAME}
#         PRIVATE
#         -fsanitize=address -g -fno-omit-frame-pointer -fno-optimize-sibling-calls
#         -fsanitize=undefined -g -fno-omit-frame-pointer
# )
# target_link_options(${TESTS_NAME}
#         PRIVATE
#         -fsanitize=address
#         -fsanitize=undefined
# )