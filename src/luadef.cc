#include "lyddef.h"
#include <iostream>

std::ostream &dbg = std::cout;

bool operator==(const LuaValue &v1, const LuaValue &v2)
{
    if (v1.kind != v2.kind)
        return false;
    if (v1.kind == LuaType::LVNil)
        return true;
    else if (v1.kind == LuaType::LVNumber)
        return v1.data.n == v2.data.n;
    else if (v1.kind == LuaType::LVBool)
        return v1.data.b == v2.data.b;
    else
        return v1.data.ptr == v2.data.ptr;
}
bool operator!=(const LuaValue &v1, const LuaValue &v2)
{
    return !(v1 == v2);
}

LuaValue::LuaValue()
{
    this->kind = LuaType::LVNil;
}

void crash(string message)
{
    std::cerr << "LUAYED CRASH: " << message << "\n";
    exit(1);
}