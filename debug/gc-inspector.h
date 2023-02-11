#ifndef GC_INSPECTOR_H
#define GC_INSPECTOR_H

#include "runtime.h"
#include <map>

namespace luayed
{
    class GCInspector
    {
    private:
        string cur_label = "";
        std::map<void *, int> refmap;
        int id = 1;
        int get_id(void *ptr);

    public:
        void child(void *ptr, AllocType ty, bool is_new);
        void obj(void *ptr);
        void label(const char *label);

        void keep(void *ptr);
        void dealloc(void *ptr, AllocType ty);

        void init();
    };
};

#endif