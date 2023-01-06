#include "table.h"

int table_compare(const TableElement &a, const TableElement &b)
{
    if (a.key == b.key)
        return 0;
    else
        return 1;
}

hash_t luavalue_hash(const LuaValue &v)
{
    LuaType k = v.kind;
    char buf[9] = {k, 0, 0, 0, 0, 0, 0, 0, 0};
    size_t *d = (size_t *)(buf + 1);
    if (k == LuaType::LVBool)
        *d = v.data.b;
    else if (k == LuaType::LVNumber)
        *d = v.data.n;
    else if (k == LuaType::LVNil)
        *d = 0;
    else
        *d = (size_t)v.data.ptr;
    return adler32(buf, 9);
}

hash_t table_hash(const TableElement &e)
{
    return luavalue_hash(e.key);
}

Table::Table(LuaRuntime *rt) : vset(table_compare, table_hash, rt)
{
}
void Table::set(LuaValue key, LuaValue value)
{
    TableElement e(key, value);
    if (value.kind == LuaType::LVNil)
        this->vset.remove(e);
    else
        this->vset.insert(e);
}
LuaValue Table::get(LuaValue key)
{
    LuaValue undefined;
    TableElement e(key, undefined);
    return this->vset.get(e)->value;
}