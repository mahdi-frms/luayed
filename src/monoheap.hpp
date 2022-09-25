#ifndef MONOHEAP_HPP
#define MONOHEAP_HPP

#include <stddef.h>

class Monoheap
{
private:
    char *buffer;
    size_t size;
    size_t ptr;
    void expand(size_t size);

public:
    void *alloc(size_t size);
    void destroy();
    Monoheap();
};

#endif