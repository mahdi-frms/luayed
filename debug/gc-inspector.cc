#include "gc-inspector.h"
#include <iostream>

std::ostream &gc_dbg_output = std::cout;

void GCInspector::child(void *ptr, AllocType ty, bool is_new)
{
    int id = 0;
    if (this->refmap.find(ptr) != this->refmap.cend())
    {
        id = this->refmap[ptr];
    }
    else
    {
        id = this->id++;
        this->refmap[ptr] = id;
    }
    const char *tynames[] = {
        [AllocType::ATHook] = "hook",
        [AllocType::ATString] = "string",
        [AllocType::ATTable] = "table",
        [AllocType::ATFunction] = "function",
        [AllocType::ATBinary] = "binary",
    };
    gc_dbg_output << this->cur_label << ":\n";
    gc_dbg_output << "\t" << tynames[ty] << " (" << id << ")";
    if (!is_new)
        gc_dbg_output << " -- already visited";
    gc_dbg_output << "\n";
}
void GCInspector::obj(void *ptr)
{
    gc_dbg_output << "obj(  " << this->refmap[ptr] << " )";
}
void GCInspector::label(const char *label)
{
    this->cur_label = string(label);
}
