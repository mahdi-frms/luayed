#ifndef TABLE_H
#define TABLE_H

#include "set.h"
#include "runtime.h"
#include "hash.h"

class Table;

struct TableElement
{
    LuaValue key;
    LuaValue value;

    TableElement(LuaValue key, LuaValue value);
};

class Table
{
private:
    Set<TableElement> vset;

public:
    Table(LuaRuntime *rt);

    void set(LuaValue key, LuaValue value);
    LuaValue get(LuaValue key);
};

#endif