#include "gc-inspector.h"
#include <iostream>

std::ostream &gc_dbg_output = std::cout;

const char *inspect_linebeg = "GC-INSPECT >>>> ";

int GCInspector::get_id(void *ptr)
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
    return id;
}

void GCInspector::child(void *ptr, AllocType ty, bool is_new)
{
    int id = this->get_id(ptr);

    const char *tynames[] = {
        [AllocType::ATHook] = "hook",
        [AllocType::ATString] = "string",
        [AllocType::ATTable] = "table",
        [AllocType::ATFunction] = "function",
        [AllocType::ATBinary] = "binary",
    };

    gc_dbg_output << inspect_linebeg
                  << "as " << this->cur_label << ":\n";
    gc_dbg_output << inspect_linebeg
                  << "  " << tynames[ty] << " (" << id << ")";
    if (ty == AllocType::ATString)
        gc_dbg_output << " '"
                      << ((lstr_p)ptr)->cstr()
                      << "' ";
    if (!is_new)
        gc_dbg_output << " -- already visited";
    gc_dbg_output << "\n";
}
void GCInspector::obj(void *ptr)
{
    gc_dbg_output << inspect_linebeg << "\n";
    gc_dbg_output << inspect_linebeg << "expanding (" << this->refmap[ptr] << ")\n";
}
void GCInspector::label(const char *label)
{
    this->cur_label = string(label);
}

void GCInspector::init()
{
    gc_dbg_output << inspect_linebeg << "runtime\n";
    this->id = 1;
    this->refmap.clear();
    this->cur_label = "";
}