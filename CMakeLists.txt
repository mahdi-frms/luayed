cmake_minimum_required(VERSION 3.16)

project(luayed VERSION 0.1.0)

add_executable(luayed
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/monoheap.cpp
    src/resolve.cpp
)

add_executable(luaytest
    test/test.cpp
    lib/tap/tap.c
)

enable_testing()
add_test(NAME "luayedtests" COMMAND luaytest)

target_include_directories(luaytest PRIVATE lib)

set_property(TARGET luayed PROPERTY CXX_STANDARD 17)