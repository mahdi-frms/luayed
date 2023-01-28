#ifndef GC_INSPECTOR_H
#define GC_INSPECTOR_H

#include "../src/runtime.h"
#include <map>

class GCInspector
{
private:
    string cur_label = "";
    std::map<void *, int> refmap;
    int id = 1;

public:
    void child(void *ptr, AllocType ty, bool is_new);
    void obj(void *ptr);
    void label(const char *label);
};

#endif