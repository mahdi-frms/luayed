#ifndef GC_H
#define GC_H

#include "runtime.h"
#include "table.h"

namespace luayed
{
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
        void mark();
        void sweep();

    public:
        GarbageCollector();
        void run(LuaRuntime *rt);
    };
};

#endif