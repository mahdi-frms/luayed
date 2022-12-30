#include <luadef.h>
#include <luabin.h>
#include <runtime.h>
#include <interpreter.h>
#include <generator.h>
#include <generator.h>
#include "test.h"

#define IPTR(I) Opcode(0x00, I)

LuaValue vnumber(lnumber n)
{
    LuaValue v;
    v.kind = LuaType::LVNumber;
    v.data.n = n;
    return v;
}
LuaValue vstring(const char *s)
{
    LuaValue v;
    v.kind = LuaType::LVString;
    v.data.ptr = (void *)s;
    return v;
}
LuaValue vbool(bool b)
{
    LuaValue v;
    v.kind = LuaType::LVBool;
    v.data.b = b;
    return v;
}
LuaValue vnil()
{
    LuaValue v;
    v.kind = LuaType::LVNil;
    return v;
}

void interpetor_test_case(
    const char *message,
    vector<Opcode> text,
    vector<LuaValue> rodata = {},
    vector<LuaValue> stack = {})
{
    string mes;
    mes.append("interpreter : ");
    mes.append(message);
    Interpreter::optable_init();
    Interpreter intrp;
    LuaRuntime rt((IInterpreter *)&intrp);
    // init stack
    for (size_t i = 0; i < stack.size(); i++)
    {
        rt.stack_push(stack[i]);
    }
    // execute instructions
    for (size_t i = 0; i < text.size(); i++)
    {
        intrp.run(&rt, text[i]);
    }
    LuaValue top = rt.stack_pop();
    test_assert(top.kind == LuaType::LVBool && top.data.b, mes.c_str());
}

void interpreter_tests()
{
    interpetor_test_case("push true", {itrue}, {}, {});
}
