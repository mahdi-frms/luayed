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

set_property(TARGET luayed PROPERTY CXX_STANDARD 17)