#include "gc-inspector.h"
#include <iostream>

using namespace luayed;

std::ostream &gc_dbg_output = std::cout;

const char *inspect_linebeg = "GC-INSPECT >>>> ";

const char *atynames[] = {
    [AllocType::ATHook] = "hook",
    [AllocType::ATString] = "string",
    [AllocType::ATTable] = "table",
    [AllocType::ATFunction] = "function",
    [AllocType::ATBinary] = "binary",
};

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

    gc_dbg_output << inspect_linebeg
                  << "as " << this->cur_label << ":\n";
    gc_dbg_output << inspect_linebeg
                  << "  " << atynames[ty] << " (" << id << ")";
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
void GCInspector::dealloc(void *ptr, AllocType ty)
{
    gc_dbg_output << inspect_linebeg;
    gc_dbg_output << "freeing ";
    gc_dbg_output << atynames[ty];
    if (ty == AllocType::ATString)
        gc_dbg_output << " '"
                      << ((lstr_p)ptr)->cstr()
                      << "' ";
    gc_dbg_output << "\n";
}
void GCInspector::keep(void *ptr)
{
    gc_dbg_output << inspect_linebeg;
    gc_dbg_output << "keeping (" << this->refmap[ptr] << ")\n";
}
