cmake_minimum_required(VERSION 3.16)

project(luayed VERSION 0.1.0)

add_executable(luayed
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/compiler.cpp
    src/ast.cpp
    src/monoheap.cpp
    src/resolve.cpp
    src/runtime.cpp
    src/generator.cpp
    src/interpretor.cpp
    src/luabin.cpp
    )
    
    add_executable(luaytest
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/monoheap.cpp
    src/compiler.cpp
    src/generator.cpp
    src/runtime.cpp
    src/resolve.cpp
    src/luabin.cpp
    test/lexer.cpp
    test/compiler.cpp
    test/test.cpp
    test/parser.cpp
    lib/tap/tap.c
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall")

enable_testing()
add_test(NAME "luayedtests" COMMAND luaytest)

target_include_directories(luaytest PRIVATE lib)
target_include_directories(luaytest PRIVATE src)

set_property(TARGET luayed PROPERTY CXX_STANDARD 17)