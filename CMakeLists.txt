cmake_minimum_required(VERSION 3.18)

project(a65n LANGUAGES C)

option(SANITIZE "Compile with asan/ubsan (gcc/clang only)" OFF)

add_executable(a65n
    src/a65.c
    src/a65.h
    src/a65eval.c
    src/a65eval.h
    src/a65util.c
    src/a65util.h
)

target_include_directories(a65n PRIVATE src)

set_target_properties(a65n PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(a65n PRIVATE /W3)
else()
    target_compile_options(a65n PRIVATE -Wall -Wvla -Wformat=2 -MMD)
    if (SANITIZE)
        target_compile_options(a65n PRIVATE -fsanitize=address,undefined)
        target_link_options(a65n PRIVATE -fsanitize=address,undefined)
    endif()
endif()
