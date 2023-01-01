#ifndef LUA_h
#define LUA_CPP

#include "runtime.h"
#include "interpreter.h"

class Lua;

typedef size_t (*LuaCppFunction)(Lua *);

class Lua
{
private:
    LuaRuntime runtime;
    Interpreter interpreter;

public:
    Lua();
    void compile(const char *lua_code);
    void push_cppfn(LuaCppFunction cppfn);
    lnumber pop_number();
    void call(size_t arg_count, size_t return_count);
};

#endif