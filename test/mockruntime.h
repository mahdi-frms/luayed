#ifndef MOCK_RUNTIME_H
#define MOCK_RUNTIME_H

#include "virtuals.h"

namespace luayed
{

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

    LuaValue lvnil();
    LuaValue lvbool(bool b);
    LuaValue lvnumber(lnumber n);
    LuaValue lvstring(const char *s);
    LuaValue lvtable();

    class MockRuntime : public IRuntime
    {
    private:
        vector<LuaValue> stack;
        vector<LuaValue> constants;
        vector<LuaValue> args;
        vector<lbyte> instructions;
        vector<Hook> upvalue_hooks;
        vector<LuaValue> upvalues;
        LuaValue error = lvnil();
        LuaValue global = lvtable();
        bool has_error = false;
        size_t ip = 0;

        size_t back_stack(size_t idx);

    public:
        Intercept icp_luafn;
        Intercept icp_fncall;
        Intercept icp_hookpush;
        Intercept icp_hookpop;

        void set_stack(vector<LuaValue> stack);
        void set_constants(vector<LuaValue> constants);
        void set_args(vector<LuaValue> args);
        void set_text(vector<Bytecode> text);
        bool compare_stack(vector<LuaValue> values);

        void table_set(LuaValue t, LuaValue k, LuaValue v);
        LuaValue table_get(LuaValue t, LuaValue k);
        LuaValue table_global();

        void add_upvalue(LuaValue value);
        void add_detached_upvalue(LuaValue value);

        vector<LuaValue> &get_stack();

        LuaValue create_nil();
        LuaValue create_boolean(bool b);
        LuaValue create_number(lnumber n);
        LuaValue create_string(const char *s);
        LuaValue create_string(lnumber n);
        LuaValue create_string(const char *s1, const char *s2);
        LuaValue create_table();
        LuaValue create_luafn(fidx_t fidx);
        LuaValue stack_pop();
        void stack_push(LuaValue value);
        LuaValue stack_read(size_t idx);
        void stack_write(size_t idx, LuaValue value);
        LuaValue stack_back_read(size_t idx);
        void stack_back_write(size_t idx, LuaValue value);
        void hookpush();
        void hookpop();
        LuaValue arg(size_t idx);
        Hook *upvalue(size_t idx);
        LuaValue rodata(size_t idx);
        lbyte *text();
        void set_error(LuaValue value);
        LuaValue get_error();
        bool error_raised();
        bool error_metadata();
        void error_metadata(bool md);
        size_t extras();
        void extras(size_t count);
        size_t argcount();
        uint16_t *dbgmd();
        LuaValue chunkname();
        void check_garbage_collection();
        void store_ip(size_t ip);
        size_t load_ip();
        size_t length(const char *str);
    };
};

#endif