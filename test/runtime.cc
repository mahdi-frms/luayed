#include <runtime.h>
#include "test.h"

void rt_assert(bool rsl, const char *message, size_t idx = 0)
{
    string mes = "runtime : ";
    mes.append(message);
    if (idx)
    {
        mes.append(" [");
        mes.append(std::to_string(idx));
        mes.append("]");
    }
    test_assert(rsl, mes.c_str());
}

void test_pushpop()
{
    const char *mes = "stack operations";
    LuaRuntime rt(nullptr);
    vector<LuaValue> values = {
        lvnil(),
        lvnumber(200),
        lvnumber(13),
        lvnil(),
        lvbool(true),
    };

    rt_assert(rt.stack_size() == 0, mes, 1);

    for (size_t i = 0; i < values.size(); i++)
        rt.stack_push(values[i]);

    rt_assert(rt.stack_size() == values.size(), mes, 2);

    vector<LuaValue> stack;
    while (rt.stack_size())
    {
        LuaValue v = rt.stack_pop();
        stack.insert(stack.begin(), v);
    }
    rt_assert(values == stack, mes, 3);
}

void test_create_values()
{
    const char *mes = "value creation";
    LuaRuntime rt(nullptr);

    LuaValue values[3] = {
        rt.create_nil(),
        rt.create_number(11),
        rt.create_boolean(false),
    };

    rt_assert(values[0].kind == LuaType::LVNil, mes, 1);
    rt_assert(values[1].kind == LuaType::LVNumber, mes, 2);
    rt_assert(values[1].data.n == 11, mes, 3);
    rt_assert(values[2].kind == LuaType::LVBool, mes, 4);
    rt_assert(values[2].data.b == false, mes, 5);
}

void runtime_tests()
{
    test_pushpop();
    test_create_values();
}