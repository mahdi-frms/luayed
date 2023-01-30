#include "gc.h"

#define gcheadptr(GCH, T) ((T *)(GCH + 1))

#ifdef GC_DEBUG
#include "../debug/gc-inspector.h"
GCInspector inspector;
#endif

void GarbageCollector::scan(gc_header_t *obj)
{
#ifdef GC_DEBUG
    inspector.obj(obj + 1);
#endif
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

#ifdef GC_DEBUG
        inspector.label("included binary");
#endif
        this->reference(this->rt->bin(fidx));
    }
    for (size_t i = 0; i < bin->rolen; i++)
    {
#ifdef GC_DEBUG
        inspector.label("constant");
#endif
        this->value(bin->rodata()[i]);
    }
#ifdef GC_DEBUG
    inspector.label("chunck name");
#endif
    this->value(bin->chunkname);
}
void GarbageCollector::scan(LuaFunction *fn)
{
    if (fn->is_lua)
    {

#ifdef GC_DEBUG
        inspector.label("function binary");
#endif
        this->reference(fn->fn);
        Lfunction *bin = fn->binary();
        Hook **hooks = (Hook **)(fn + 1);
        size_t uplen = bin->uplen;
        for (size_t i = 0; i < uplen; i++)
        {
#ifdef GC_DEBUG
            inspector.label("function hooks");
#endif
            this->reference(hooks[i]);
        }
    }
}
void GarbageCollector::scan(Table *table)
{
    TableIterator it = table->iter();
    while (it.next())
    {
        LuaValue key = it.key();
        LuaValue value = it.value();

#ifdef GC_DEBUG
        inspector.label("table key");
#endif
        this->value(key);
#ifdef GC_DEBUG
        inspector.label("table value");
#endif
        this->value(value);
    }
}
void GarbageCollector::scan(Hook *hook)
{
    if (hook->is_detached)
    {
#ifdef GC_DEBUG
        inspector.label("detached hook value");
#endif
        this->value(hook->val);
    }
    else
    {
#ifdef GC_DEBUG
        inspector.label("pointed hook value");
#endif
        this->value(*hook->original);
    }
}
void GarbageCollector::scan()
{
    Frame *frame = rt->topframe();
    while (frame)
    {
        if (frame->has_error)
        {
#ifdef GC_DEBUG
            inspector.label("frame error");
#endif
            this->value(frame->error);
        }
#ifdef GC_DEBUG
        inspector.label("frame function");
#endif
        this->value(frame->fn);
        for (size_t i = 0; i < frame->hookptr; i++)
        {
            Hook *hook = frame->hooktable()[i];
            if (hook)
            {
#ifdef GC_DEBUG
                inspector.label("hook lifo");
#endif
                this->reference(hook);
            }
        }
        for (size_t i = 0; i < frame->sp; i++)
        {
#ifdef GC_DEBUG
            inspector.label("stack value");
#endif
            this->value(frame->stack()[i]);
        }
        frame = frame->prev;
    }

#ifdef GC_DEBUG
    inspector.label("gobal table");
#endif
    this->value(rt->table_global());
}
void GarbageCollector::mark()
{
#ifdef GC_DEBUG
    inspector.init();
#endif
    this->scan();
    while (this->scanlifo->alloc_type != AllocType::ATDummy)
    {
        gc_header_t *obj = this->scanlifo;
        this->scanlifo = obj->scan;
        this->scan(obj);
    }
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
#ifdef GC_DEBUG
    inspector.child(ptr, header->alloc_type, !header->marked);
#endif
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
void GarbageCollector::sweep()
{
    gc_header_t *hdptr = this->rt->gc_headers()->next;
    while (hdptr->alloc_type != AllocType::ATDummy)
    {
        gc_header_t *next = hdptr->next;
        if (hdptr->marked)
        {

#ifdef GC_DEBUG
            inspector.keep(hdptr + 1);
#endif
            hdptr->marked = false;
        }
        else
        {
#ifdef GC_DEBUG
            inspector.dealloc(hdptr + 1, hdptr->alloc_type);
#endif
            this->rt->deallocate(hdptr);
        }
        hdptr = next;
    }
}

void GarbageCollector::run(LuaRuntime *rt)
{
    this->rt = rt;
    this->mark();
    this->sweep();
    this->rt = nullptr;
}