#ifndef LUA_h
#define LUA_CPP

#include "runtime.h"
#include "interpretor.h"

class Lua
{
private:
    LuaRuntime runtime;
    Interpretor interpretor;

public:
    Lua();
    void compile(const char *lua_code);
    void push_cppfn(LuaCppFunction cppfn);
    lnumber pop_number();
    void call(size_t arg_count, size_t return_count);
};

#endif