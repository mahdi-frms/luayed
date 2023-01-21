#include "gc.h"

#define gcheadptr(GCH, T) ((T *)(GCH + 1))
#define gcptrhead(PTR) (((gc_header *)PTR) - 1)

void GarbageCollector::scan(gc_header *obj)
{
    if (!obj->marked)
    {
        obj->marked = true;
        if (obj->alloc_type == AllocType::ATHook)
            this->scan(gcheadptr(obj, Hook));
        if (obj->alloc_type == AllocType::ATTable)
            this->scan(gcheadptr(obj, Table));
        if (obj->alloc_type == AllocType::ATFunction)
            this->scan(gcheadptr(obj, LuaFunction));
        if (obj->alloc_type == AllocType::ATBinary)
            this->scan(gcheadptr(obj, Lfunction));
    }
}
void GarbageCollector::scan(Lfunction *bin)
{
    for (size_t i = 0; i < bin->inlen; i++)
    {
        size_t fidx = bin->innerfns()[i];
        this->reference(this->rt->bin(fidx));
    }
    for (size_t i = 0; i < bin->rolen; i++)
        this->value(bin->rodata()[i]);
}
void GarbageCollector::scan(LuaFunction *fn)
{
    if (fn->is_lua)
    {
        this->reference(fn->fn);
        Lfunction *bin = fn->binary();
        Hook **hooks = (Hook **)(fn + 1);
        size_t uplen = bin->uplen;
        for (size_t i = 0; i < uplen; i++)
            this->reference(hooks[i]);
    }
}
void GarbageCollector::scan(Table *table)
{
    TableIterator it = table->iter();
    while (it.next())
    {
        LuaValue key = it.key();
        LuaValue value = it.value();
        this->value(key);
        this->value(value);
    }
}
void GarbageCollector::scan(Hook *hook)
{
    if (hook->is_detached)
        this->value(*hook->original);
    else
        this->value(hook->val);
}
void GarbageCollector::scan(LuaRuntime *rt)
{
    this->rt = rt;
    Frame *frame = rt->topframe();
    while (frame)
    {
        if (frame->has_error)
            this->value(frame->error);
        this->value(frame->fn);
        for (size_t i = 0; i < frame->hookptr; i++)
            this->reference(frame->hooktable()[i]);
        for (size_t i = 0; i < frame->sp; i++)
            this->value(frame->stack()[i]);
        frame = frame->prev;
    }
    this->rt = nullptr;
}
void GarbageCollector::value(LuaValue val)
{
    if (val.kind >= 3)
    {
        this->reference(val.data.ptr);
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
    this->rt = nullptr;
    this->dummy.scan =
        this->dummy.next =
            this->dummy.prev = nullptr;

    this->dummy.marked = true;
    this->dummy.scan = nullptr;
    this->dummy.alloc_type = AllocType::ATDummy;
}
