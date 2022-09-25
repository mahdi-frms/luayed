#include "monoheap.hpp"
#include <cstdlib>
#include <stdio.h>

size_t max(size_t a, size_t b)
{
    return a > b ? a : b;
}

#include "ast.hpp"

void *Monoheap::alloc(size_t size)
{
    size_t rem = this->size - this->ptr;
    if (size > rem)
    {
        this->expand(max(size, this->size * 2));
    }
    void *ptr = this->buffer + sizeof(size_t) + this->ptr;
    this->ptr += size;
    return ptr;
}

void Monoheap::expand(size_t size)
{
    char *new_buffer = (char *)malloc(size + sizeof(size_t));
    char **next = (char **)new_buffer;
    *next = this->buffer;
    this->buffer = new_buffer;
    this->ptr = 0;
    this->size = size;
}

void Monoheap::destroy()
{
    while (this->buffer)
    {
        char *next = *(char **)this->buffer;
        free(this->buffer);
        this->buffer = next;
    }
}

Monoheap::Monoheap()
{
    this->ptr = 0;
    this->size = 0;
    this->buffer = NULL;
}