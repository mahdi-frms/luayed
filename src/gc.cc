#include "gc.h"

#define gcheadptr(GCH, T) ((T *)(GCH + 1))

void GarbageCollector::scan(gc_header_t *obj)
{
    if (obj->alloc_type == AllocType::ATHook)
        this->scan(gcheadptr(obj, Hook));
    else if (obj->alloc_type == AllocType::ATTable)
        this->scan(gcheadptr(obj, Table));
    else if (obj->alloc_type == AllocType::ATFunction)
        this->scan(gcheadptr(obj, LuaFunction));
    else if (obj->alloc_type == AllocType::ATBinary)
        this->scan(gcheadptr(obj, Lfunction));
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
    this->value(bin->chunckname);
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
        this->value(hook->val);
    else
        this->value(*hook->original);
}
void GarbageCollector::scan()
{
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
    this->value(rt->table_global());
}
void GarbageCollector::mark(LuaRuntime *rt)
{
    this->rt = rt;
    this->scan();
    while (this->scanlifo->alloc_type != AllocType::ATDummy)
    {
        gc_header_t *obj = this->scanlifo;
        this->scanlifo = obj->scan;
        this->scan(obj);
    }
    this->rt = nullptr;
}
void GarbageCollector::value(LuaValue val)
{
    if (val.kind == LuaType::LVString)
        this->reference(((lstr_p)val.data.ptr) - 1);
    else if (is_obj(val))
    {
        this->reference(val.data.ptr);
    }
}
void GarbageCollector::reference(void *ptr)
{
    gc_header_t *header = ((gc_header_t *)ptr) - 1;
    if (header->marked)
        return;
    header->marked = true;
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
    this->dummy.alloc_type = AllocType::ATDummy;
    this->scanlifo = &this->dummy;
}
void GarbageCollector::sweep(LuaRuntime *rt)
{
    gc_header_t *hdptr = rt->gc_headers()->next;
    while (hdptr->alloc_type != AllocType::ATDummy)
    {
        gc_header_t *next = hdptr->next;
        if (hdptr->marked)
        {
            hdptr->marked = false;
        }
        else
        {
            rt->deallocate(hdptr);
        }
        hdptr = next;
    }
}