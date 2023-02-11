#include "test.h"
#include "values.h"
#include <tap/tap.h>

using namespace luayed;

void lexer_tests();
void parser_tests();
void compiler_tests();
void interpreter_tests();
void lua_tests();
void runtime_tests();

void luayed::test_assert(bool result, const char *message)
{
    ok(result, message);
}

vector<LuaValue> luayed::drain(LuaRuntime *rt)
{
    vector<LuaValue> stack;
    while (rt->stack_size())
        stack.insert(stack.begin(), rt->stack_pop());
    return stack;
}

LuaValue luayed::lvclone(LuaRuntime *rt, const LuaValue &v)
{
    if (v.kind == LuaType::LVString)
        return rt->create_string(v.as<const char *>());
    if (v.kind == LuaType::LVTable)
        return rt->create_table();
    else
        return v;
}

void luayed::pipe(LuaRuntime *rt, vector<LuaValue> values)
{
    for (size_t i = 0; i < values.size(); i++)
    {
        rt->stack_push(luayed::lvclone(rt, values[i]));
    }
}

void finalize_tests()
{
    tabset_detroy();
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

    finalize_tests();

    done_testing();
    return 0;
}