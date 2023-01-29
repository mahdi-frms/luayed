#include "lua.h"
#include "runtime.h"
#include "interpreter.h"

class LuaState : public Lua
{
private:
    LuaRuntime runtime;
    Interpreter interpreter;

public:
    LuaState(LuaConfig conf);
    int compile(const char *lua_code, string &errors, const char *chunkname = nullptr);
    void push_cppfn(LuaCppFunction cppfn);
    void push_string(const char *str);
    void push_nil();
    void push_number(lnumber num);
    void push_boolean(bool b);
    void insert(size_t index);
    void call(size_t arg_count, size_t return_count);
    int kind();

    void set_global();

    void set_table();
    void get_table();

    void pop();
    size_t top();
    lnumber pop_number();
    bool pop_boolean();
    const char *peek_string();
    void fetch_local(int idx);
    void store_local(int idx);

    bool has_error();
    void push_error();
    void pop_error();
};
