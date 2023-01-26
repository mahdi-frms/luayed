cmake_minimum_required(VERSION 3.16)

project(luayed VERSION 0.1.0)

add_custom_command(
    OUTPUT liblua.cc
    COMMAND lua ../scripts/loadlib.lua
    DEPENDS scripts/loadlib.lua
)

add_library(luacomp
    src/lexer.cc
    src/ast.cc
    src/parser.cc
    src/resolve.cc
    src/compiler.cc
    src/luabin.cc
    src/lerror.cc
    src/token.cc
    src/luadef.cc
    src/interpreter.cc
    src/lstrep.cc
    src/luadef.cc
    src/runtime.cc
    src/gc.cc
    src/generator.cc
    src/table.cc
    src/hash.cc
)
    
add_executable(luayed
    build/liblua.cc    
    src/main.cc
    src/luastd.cc
    src/lua.cc
)
    
add_executable(luaytest
    test/lexer.cc
    test/compiler.cc
    test/interpreter.cc
    test/mockruntime.cc
    test/runtime.cc
    test/test.cc
    test/parser.cc
    test/lua.cc
    test/values.cc
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