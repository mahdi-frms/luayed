#include "table.h"

using namespace luayed;

TableElement::TableElement(LuaValue key, LuaValue value) : key(key), value(value)
{
}

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

void Table::init(LuaRuntime *rt)
{
    this->vset.init(table_compare, table_hash, rt);
}
void Table::destroy()
{
    this->vset.destroy();
}
TableIterator Table::iter() const
{
    return TableIterator(this);
}

void Table::set(LuaValue key, LuaValue value)
{
    TableElement e(key, value);
    if (value.kind == LuaType::LVNil)
        this->vset.remove(e);
    else
        this->vset.insert(e);
}
LuaValue Table::get(LuaValue key) const
{
    LuaValue nil;
    nil.kind = LuaType::LVNil;
    TableElement e(key, nil);
    TableElement *ep = this->vset.get(e);
    if (ep)
        return ep->value;
    else
        return nil;
}
TableElement *Table::next(int &idx) const
{
    return this->vset.iter(idx);
}
TableIterator::TableIterator(const Table *table) : table(table)
{
}
bool TableIterator::next()
{
    this->el = this->table->next(this->idx);
    return this->el != nullptr;
}
LuaValue TableIterator::key() const
{
    return this->el->key;
}
LuaValue TableIterator::value() const
{
    return this->el->value;
}