#ifndef MOCK_RUNTIME_H
#define MOCK_RUNTIME_H

#include "virtuals.h"

class Intercept
{
private:
    bool used = false;
    size_t arg1 = 0;
    size_t arg2 = 0;

public:
    void enable();
    void enable(size_t arg1);
    void enable(size_t arg1, size_t arg2);
    bool check(size_t arg1, size_t arg2);
};

class MockRuntime : public IRuntime
{
private:
    vector<LuaValue> stack;
    vector<LuaValue> constants;
    vector<LuaValue> args;
    vector<lbyte> instructions;
    size_t ip;

    size_t back_stack(size_t idx);

public:
    Intercept icp_luafn;
    Intercept icp_fncall;
    Intercept icp_hookpush;
    Intercept icp_hookpop;

    void set_stack(vector<LuaValue> stack);
    void set_constants(vector<LuaValue> constants);
    void set_args(vector<LuaValue> args);
    void set_text(vector<Opcode> text);
    bool compare_stack(vector<LuaValue> values);

    vector<LuaValue> &get_stack();

    LuaValue create_nil();
    LuaValue create_boolean(bool b);
    LuaValue create_number(lnumber n);
    LuaValue create_string(const char *s);
    LuaValue create_table();
    LuaValue create_luafn(fidx_t fidx);
    void fncall(size_t argc, size_t retc);
    LuaValue stack_pop();
    void stack_push(LuaValue value);
    LuaValue stack_read(size_t idx);
    void stack_write(size_t idx, LuaValue value);
    LuaValue stack_back_read(size_t idx);
    void stack_back_write(size_t idx, LuaValue value);
    void hookpush();
    void hookpop();
    LuaValue arg(size_t idx);
    size_t load_ip();
    void save_ip(size_t ip);
    Hook *upvalue(size_t idx);
    LuaValue rodata(size_t idx);
    lbyte *text();
};

LuaValue lvnil();
LuaValue lvbool(bool b);
LuaValue lvnumber(lnumber n);
LuaValue lvstring(const char *s);
LuaValue lvtable();

#endif