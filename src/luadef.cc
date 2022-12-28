#include "luadef.h"
#include <iostream>

const char *LuaValue::as_string()
{
    return (const char *)this->data.ptr;
}
bool LuaValue::truth()
{
    return this->kind != LuaType::LVNil && (this->kind != LuaType::LVBool || this->data.b);
}
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
std::ostream &operator<<(std::ostream &strm, const LuaValue &v)
{
    strm << v.kind;
    if (v.kind == LuaType::LVBool)
        strm << '(' << (v.data.b ? "true" : "false") << ')';
    else if (v.kind == LuaType::LVNumber)
        strm << '(' << v.data.n << ')';
    else if (v.kind == LuaType::LVString)
        strm << '(' << (const char *)v.data.ptr << ')';
    return strm;
}
std::ostream &operator<<(std::ostream &strm, const LuaType &t)
{
    if (t == LuaType::LVNil)
        strm << "nil";
    else if (t == LuaType::LVBool)
        strm << "bool";
    if (t == LuaType::LVNumber)
        strm << "number";
    if (t == LuaType::LVTable)
        strm << "table";
    if (t == LuaType::LVString)
        strm << "string";
    if (t == LuaType::LVFunction)
        strm << "function";
    return strm;
}