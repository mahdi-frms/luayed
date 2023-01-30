#ifndef LUA_H
#define LUA_H

#define LUA_COMPILE_RESULT_OK 0
#define LUA_COMPILE_RESULT_FAILED 1

#define LUA_TYPE_NIL 0
#define LUA_TYPE_BOOLEAN 1
#define LUA_TYPE_NUMBER 2
#define LUA_TYPE_STRING 3
#define LUA_TYPE_TABLE 4
#define LUA_TYPE_FUNCTION 5

#include "luadef.h"

struct LuaConfig
{
    bool load_stdlib = true;
};

class Lua;

typedef size_t (*LuaCppFunction)(Lua *);

class Lua
{
public:
    static Lua *create(LuaConfig conf = LuaConfig());
    virtual int compile(const char *lua_code, string &errors, const char *chunkname = nullptr) = 0;
    virtual void push_cppfn(LuaCppFunction cppfn) = 0;
    virtual void push_string(const char *str) = 0;
    virtual void push_nil() = 0;
    virtual void push_number(lnumber num) = 0;
    virtual void push_boolean(bool b) = 0;
    virtual void insert(size_t index) = 0;
    virtual void call(size_t arg_count, size_t return_count) = 0;
    virtual int kind() = 0;
    virtual void set_global(const char *key) = 0;
    virtual void set_table() = 0;
    virtual void get_table() = 0;
    virtual void pop() = 0;
    virtual size_t top() = 0;
    virtual lnumber pop_number() = 0;
    virtual bool pop_boolean() = 0;
    virtual const char *peek_string() = 0;
    virtual void fetch_local(int idx) = 0;
    virtual void store_local(int idx) = 0;
    virtual bool has_error() = 0;
    virtual void push_error() = 0;
    virtual void pop_error() = 0;
};

#endif