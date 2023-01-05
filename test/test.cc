#include "test.h"
#include <tap/tap.h>

void lexer_tests();
void parser_tests();
void compiler_tests();
void interpreter_tests();
void lua_tests();
void runtime_tests();

void test_assert(bool result, const char *message)
{
    ok(result, message);
}

vector<LuaValue> drain(LuaRuntime *rt)
{
    vector<LuaValue> stack;
    while (rt->stack_size())
        stack.insert(stack.begin(), rt->stack_pop());
    return stack;
}

void pipe(LuaRuntime *rt, vector<LuaValue> values)
{
    for (size_t i = 0; i < values.size(); i++)
    {
        LuaValue v = values[i];
        LuaValue nv = rt->create_nil();
        if (v.kind == LuaType::LVNumber)
            nv = rt->create_number(v.data.n);
        else if (v.kind == LuaType::LVBool)
            nv = rt->create_boolean(v.data.b);
        else if (v.kind == LuaType::LVString)
            nv = rt->create_string(v.as<const char *>());
        else if (v.kind == LuaType::LVTable)
            nv = rt->create_table();
        else
            nv = v;
        rt->stack_push(nv);
    }
}

int main()
{
    plan(NO_PLAN);

    lexer_tests();
    parser_tests();
    compiler_tests();
    interpreter_tests();
    runtime_tests();
    lua_tests();

    done_testing();
    return 0;
}