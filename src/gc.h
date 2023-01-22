#ifndef GC_H
#define GC_H

#include "runtime.h"
#include "table.h"

class GarbageCollector : public IGarbageCollector
{
    LuaRuntime *rt;
    gc_header_t *scanlifo;
    gc_header_t dummy;

    void scan(gc_header_t *obj);
    void scan(Hook *hook);
    void scan(Table *table);
    void value(LuaValue val);
    void scan(LuaFunction *fn);
    void scan(Lfunction *fn);
    void reference(void *ptr);
    void scan();

public:
    GarbageCollector();
    void mark(LuaRuntime *rt);
    void sweep(LuaRuntime *rt);
};

#endif