#ifndef GC_H
#define GC_H

#include "runtime.h"
#include "table.h"

class GarbageCollector : public IGarbageCollector
{
    LuaRuntime *rt;
    gc_header *scanlifo;
    gc_header dummy;

    void scan(gc_header *obj);
    void scan(Hook *hook);
    void scan(Table *table);
    void value(LuaValue val);
    void scan(LuaFunction *fn);
    void scan(Lfunction *fn);
    void reference(void *ptr);

public:
    GarbageCollector();
    void scan(LuaRuntime *rt);
};

#endif