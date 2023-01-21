#ifndef GC_H
#define GC_H

#include "runtime.h"

class GarbageCollector : public IGarbageCollector
{
public:
    void collect(LuaRuntime *rt);
    void reference(void *ptr);
};

#endif