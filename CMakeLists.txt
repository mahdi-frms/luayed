cmake_minimum_required(VERSION 3.16)

project(luayed VERSION 0.1.0)

add_library(luacomp
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/resolve.cpp
    src/compiler.cpp
    src/luabin.cpp
    src/lerror.cpp
    src/token.cpp
)

add_executable(luayed
    src/main.cpp
    src/runtime.cpp
    src/generator.cpp
    src/interpretor.cpp
    src/lerror.cpp
    src/lua.cpp
)
    
add_executable(luaytest
    test/lexer.cpp
    test/compiler.cpp
    test/test.cpp
    test/parser.cpp
    lib/tap/tap.c
)

target_link_libraries(luayed luacomp)
target_link_libraries(luaytest luacomp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall")

enable_testing()
add_test(NAME "luayedtests" COMMAND luaytest)

target_include_directories(luaytest PRIVATE lib)
target_include_directories(luaytest PRIVATE src)

set_property(TARGET luayed PROPERTY CXX_STANDARD 17)