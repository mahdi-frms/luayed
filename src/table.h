#ifndef TABLE_H
#define TABLE_H

#include "set.h"
#include "runtime.h"
#include "hash.h"

class Table;
class TableIterator;

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
    void clean();
    void init(LuaRuntime *rt);
    void destroy();

    void set(LuaValue key, LuaValue value);
    LuaValue get(LuaValue key) const;
    TableElement *next(int &idx) const;
    TableIterator iter() const;
};

class TableIterator
{
private:
    const Table *table = nullptr;
    int idx = -1;
    TableElement *el = nullptr;

public:
    TableIterator(const Table *table);
    bool next();
    LuaValue key() const;
    LuaValue value() const;
};

#endif