#include "gc-inspector.h"
#include <iostream>

std::ostream &gc_dbg_output = std::cout;

void GCInspector::obj(void *ptr, AllocType ty, bool is_new)
{
    const char *tynames[] = {
        [AllocType::ATHook] = "hook",
        [AllocType::ATString] = "string",
        [AllocType::ATTable] = "table",
        [AllocType::ATFunction] = "function",
        [AllocType::ATBinary] = "binary",
    };
    gc_dbg_output << this->indent(-1) << this->cur_label << ":\n";
    gc_dbg_output << this->indent() << tynames[ty] << " (" << ptr << ")";
    if (!is_new)
        gc_dbg_output << " -- already visited";
    gc_dbg_output << "\n";
    this->depth++;
}
void GCInspector::obj()
{
    gc_dbg_output << "Runtime:\n";
    this->depth++;
}
void GCInspector::label(const char *label)
{
    this->cur_label = string(label);
}
void GCInspector::end_of_obj()
{
    this->depth--;
}
string GCInspector::indent(int indent_diff)
{
    return string(4 * this->depth - indent_diff, ' ');
}
