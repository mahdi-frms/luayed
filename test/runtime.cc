#include <runtime.h>
#include "test.h"
#include <lstrep.h>

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
        rt->stack_push(values[i]);
}

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
    rt.set_lua_interface(&rt);
    vector<LuaValue> values = {
        lvnil(),
        lvnumber(200),
        lvnumber(13),
        lvnil(),
        lvbool(true),
    };
    rt_assert(rt.stack_size() == 0, mes, 1);
    pipe(&rt, values);
    rt_assert(rt.stack_size() == values.size(), mes, 2);
    vector<LuaValue> stack = drain(&rt);
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

size_t lfcxx1(void *r)
{
    LuaRuntime *rt = (LuaRuntime *)r;
    vector<LuaValue> stack = drain(rt);
    for (size_t i = 0; i < stack.size(); i++)
        stack[i].data.n = stack[i].data.n + 2;
    stack.push_back(rt->create_boolean(true));
    stack.push_back(rt->create_boolean(false));
    pipe(rt, stack);
    return rt->stack_size() - 2;
}

void test_cxx_calls_cxx_extra()
{
    const char *mes = "CXX calls CXX returning extra args";
    LuaRuntime rt(nullptr);
    rt.set_lua_interface(&rt);
    rt.stack_push(rt.create_cppfn(lfcxx1));

    pipe(&rt,
         {
             rt.create_number(8),
             rt.create_number(15),
             rt.create_number(-44),
             rt.create_number(0),
             rt.create_number(600),
         });
    rt.fncall(5, 5);

    rt_assert(rt.stack_size() == 4, mes, 1);
    vector<LuaValue> stack = drain(&rt);
    rt_assert(stack ==
                  vector<LuaValue>({
                      rt.create_number(2),
                      rt.create_number(602),
                      rt.create_boolean(true),
                      rt.create_boolean(false),
                  }),
              mes, 2);
}

size_t lfcxx2(void *r)
{
    LuaRuntime *rt = (LuaRuntime *)r;
    rt->stack_push(rt->create_boolean(true));
    return 1;
}

void test_cxx_calls_cxx_less()
{
    const char *mes = "CXX calls CXX returning less args";
    LuaRuntime rt(nullptr);
    rt.set_lua_interface(&rt);
    rt.stack_push(rt.create_number(14));
    rt.stack_push(rt.create_cppfn(lfcxx2));

    pipe(&rt,
         {
             rt.create_number(8),
             rt.create_number(15),
             rt.create_number(-44),
             rt.create_number(0),
             rt.create_number(600),
         });
    rt.fncall(5, 5);

    rt_assert(rt.stack_size() == 5, mes, 1);
    vector<LuaValue> stack = drain(&rt);
    vector<LuaValue> expected = {
        rt.create_number(14),
        rt.create_boolean(true),
        rt.create_nil(),
        rt.create_nil(),
        rt.create_nil(),
    };
    bool rsl = stack == expected;
    rt_assert(rsl, mes, 2);
    if (!rsl)
    {
        std::cerr << "stack:\n"
                  << stack
                  << "expected:\n"
                  << expected;
    }
}

void test_cxx_calls_cxx()
{
    test_cxx_calls_cxx_extra();
    test_cxx_calls_cxx_less();
}

void test_calls()
{
    test_cxx_calls_cxx();
}

void runtime_tests()
{
    test_pushpop();
    test_create_values();
    test_calls();
}