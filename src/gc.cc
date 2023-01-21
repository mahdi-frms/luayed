#include "gc.h"

void GarbageCollector::collect(LuaRuntime *rt)
{
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
