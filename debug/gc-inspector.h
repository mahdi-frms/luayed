#ifndef GC_INSPECTOR_H
#define GC_INSPECTOR_H

#include "../src/runtime.h"

class GCInspector
{
private:
    int depth = 0;
    string cur_label = "";
    string indent(int indent_diff = 0);

public:
    void obj(void *ptr, AllocType ty, bool is_new);
    void obj();
    void label(const char *label);
    void end_of_obj();
};

#endif