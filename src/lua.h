#ifndef LUA_h
#define LUA_CPP

#include "runtime.h"
#include "interpreter.h"

#define LUA_COMPILE_RESULT_OK 0
#define LUA_COMPILE_RESULT_FAILED 1

class Lua;

typedef size_t (*LuaCppFunction)(Lua *);

class Lua
{
private:
    LuaRuntime runtime;
    Interpreter interpreter;

public:
    Lua();
    int compile(const char *lua_code, string &errors);
    void push_cppfn(LuaCppFunction cppfn);
    lnumber pop_number();
    void call(size_t arg_count, size_t return_count);
};

#endif