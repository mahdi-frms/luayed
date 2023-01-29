cmake_minimum_required(VERSION 3.16)

project(luayed VERSION 0.1.0)

add_custom_command(
    OUTPUT liblua.cc
    COMMAND lua ../scripts/loadlib.lua
    DEPENDS scripts/loadlib.lua lib/luastd.cc
)

add_library(luaystd
    lib/luastd.cc
    build/liblua.cc    
)

add_library(luaydbg
    debug/gc-inspector.cc
)

add_library(luayed
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
    src/luastate.cc
    src/lua.cc
)
    
add_executable(luaycli
    bin/main.cc
)

set_target_properties(luaycli
        PROPERTIES OUTPUT_NAME luayed)
    
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
    dep/tap/tap.c
)

target_link_libraries(luayed luaystd)
target_link_libraries(luayed debug luaydbg)
target_link_libraries(luaycli luayed)
target_link_libraries(luaytest luayed)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall")

enable_testing()
add_test(NAME "luayedtests" COMMAND luaytest)

include_directories(dep)
include_directories(src)
include_directories(include)
include_directories(lib)

set_property(TARGET luayed PROPERTY CXX_STANDARD 17)