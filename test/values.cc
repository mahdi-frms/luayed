#ifndef TEST_VALUES_H
#define TEST_VALUES_H

#include <luadef.h>
#include <set>
#include <map>
#include "values.h"

std::set<string> strset;
std::vector<vector<LuaValue> *> tabset;

void tabset_detroy()
{
    while (tabset.size())
    {
        delete tabset.back();
        tabset.pop_back();
    }
}

LuaValue lvnil()
{
    LuaValue v;
    v.kind = LuaType::LVNil;
    return v;
}
LuaValue lvbool(bool b)
{
    LuaValue v;
    v.kind = LuaType::LVBool;
    v.data.b = b;
    return v;
}
LuaValue lvnumber(lnumber n)
{
    LuaValue v;
    v.kind = LuaType::LVNumber;
    v.data.n = n;
    return v;
}
LuaValue lvstring(const char *s)
{
    string str = s;
    auto it = strset.cend();
    if ((it = strset.find(s)) != strset.cend())
    {
        s = it->c_str();
    }
    else
    {
        strset.insert(str);
        s = strset.find(str)->c_str();
    }
    LuaValue v;
    v.kind = LuaType::LVString;
    v.data.ptr = (void *)s;
    return v;
}
LuaValue lvtable()
{
    LuaValue v;
    v.kind = LuaType::LVTable;
    v.data.ptr = new vector<LuaValue>;
    return v;
}

#endif