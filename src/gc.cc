#include "gc.h"

#define gcheadptr(GCH, T) ((T *)(GCH + 1))
#define gcptrhead(PTR) (((gc_header *)PTR) - 1)

void GarbageCollector::scan(gc_header *obj)
{
    obj->marked = true;
    if (obj->alloc_type == AllocType::ATHook)
        this->scan(gcheadptr(obj, Hook));
    if (obj->alloc_type == AllocType::ATTable)
        this->scan(gcheadptr(obj, Table));
}
void GarbageCollector::scan(Table *table)
{
    TableIterator it = table->iter();
    while (it.next())
    {
        LuaValue key = it.key();
        LuaValue value = it.value();
        this->scan(&key);
        this->scan(&value);
    }
}
void GarbageCollector::scan(Hook *hook)
{
    if (hook->is_detached)
        this->scan(hook->original);
    else
        this->scan(&hook->val);
}
void GarbageCollector::scan(LuaRuntime *rt)
{
}
void GarbageCollector::scan(LuaValue *val)
{
    if (val->kind >= 3)
    {
        this->reference(gcptrhead(val->data.ptr));
    }
}
void GarbageCollector::reference(void *ptr)
{
    gc_header *header = ((gc_header *)ptr) - 1;
    header->scan = this->scanlifo;
    this->scanlifo = header;
}
GarbageCollector::GarbageCollector()
{
    this->dummy.scan =
        this->dummy.next =
            this->dummy.prev = nullptr;

    this->dummy.marked = true;
    this->dummy.scan = nullptr;
    this->dummy.alloc_type = AllocType::ATDummy;
}
