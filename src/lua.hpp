#ifndef LUA_HPP
#define LUA_CPP

#include "runtime.hpp"
#include "interpretor.hpp"

class Lua
{
private:
    LuaRuntime runtime;
    Interpretor interpretor;

public:
    Lua();
    void compile(const char *lua_code);
    void push_cppfn(LuaCppFunction cppfn);
    void call(size_t arg_count, size_t return_count);
};

#endif