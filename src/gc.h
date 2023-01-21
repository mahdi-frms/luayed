#ifndef GC_H
#define GC_H

#include "runtime.h"

class GarbageCollector : public IGarbageCollector
{
    gc_header *scanlifo;
    gc_header dummy;

public:
    GarbageCollector();
    void collect(LuaRuntime *rt);
    void reference(void *ptr);
};

#endif