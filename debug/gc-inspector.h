#ifndef GC_INSPECTOR_H
#define GC_INSPECTOR_H

#include "../src/runtime.h"

class GCInspector
{
public:
    void obj(void *ptr, AllocType ty, bool is_new);
    void label(const char *label);
    void end_of_obj();
};

#endif