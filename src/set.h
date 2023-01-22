#ifndef SET_H
#define SET_H

#include <stddef.h>
#include "virtuals.h"

typedef size_t hash_t;

template <typename T>
using key_compare = int (*)(const T &a, const T &b);

template <typename T>
using key_hash = hash_t (*)(const T &a);

#define SET_FLAG_EMPTY 0
#define SET_FLAG_FULL 1
#define SET_FLAG_DEAD 2

#define SET_LOAD_FACTOR 0.75f
#define SET_GROWTH_RATE 2

template <typename T>
struct Bucket
{
    T val;
    unsigned char flag;
};

template <typename T>
class Set
{
private:
    key_compare<T> comp;
    key_hash<T> hash;
    IAllocator *allocator;
    size_t cap;
    size_t count;
    Bucket<T> *buffer;

    size_t next_idx(size_t idx) const
    {
        return (idx + 1) % this->cap;
    }

    Bucket<T> *next(Bucket<T> *b)
    {
        if (b == this->buffer + (this->cap - 1))
            b = this->buffer;
        else
            b = b + 1;
        return b;
    }

    Bucket<T> *prev(Bucket<T> *b)
    {
        if (b == this->buffer)
            b = this->buffer + this->cap - 1;
        else
            b = b - 1;
        return b;
    }

    Bucket<T> *search(const T &ele) const
    {
        Bucket<T> *dead = nullptr;
        hash_t h = this->hash(ele) % this->cap, i = h;
        while (this->buffer[i].flag != SET_FLAG_EMPTY)
        {
            if (this->buffer[i].flag == SET_FLAG_DEAD)
            {
                if (!dead)
                    dead = this->buffer + i;
            }
            else if (this->comp(this->buffer[i].val, ele) == 0)
            {
                return this->buffer + i;
            }
            i = this->next_idx(i);
        }
        if (dead)
            return dead;
        return this->buffer + i;
    }
    double calc_load()
    {
        return ((double)this->count) / ((double)this->cap);
    }
    Bucket<T> *allocate(size_t size)
    {
        Bucket<T> *buf = (Bucket<T> *)this->allocator->allocate_raw(sizeof(Bucket<T>) * size);
        for (size_t i = 0; i < size; i++)
            buf[i].flag = SET_FLAG_EMPTY;
        return buf;
    }
    void free(Bucket<T> *buffer)
    {
        this->allocator->deallocate_raw(buffer);
    }
    void grow()
    {
        size_t oldcap = this->cap;
        this->cap = this->cap * SET_GROWTH_RATE;
        Bucket<T> *oldbuffer = this->buffer;
        this->buffer = this->allocate(this->cap);
        for (size_t i = 0; i < oldcap; i++)
        {
            if (oldbuffer[i].flag == SET_FLAG_FULL)
                this->insert(oldbuffer[i].val);
        }
        this->free(oldbuffer);
    }

public:
    void init(key_compare<T> comp, key_hash<T> hash, IAllocator *allocator, size_t cap = 16)
    {
        this->comp = comp;
        this->hash = hash;
        this->allocator = allocator;
        this->cap = cap;
        this->count = 0;
        this->buffer = this->allocate(cap);
    }
    void destroy()
    {
        this->free(buffer);
    }
    void insert(T ele)
    {
        if (this->calc_load() > SET_LOAD_FACTOR)
        {
            this->grow();
        }
        Bucket<T> *buck = this->search(ele);
        buck->val = ele;
        if (buck->flag != SET_FLAG_FULL)
        {
            buck->flag = SET_FLAG_FULL;
            this->count++;
        }
    }
    void remove(const T &ele)
    {
        Bucket<T> *buck = this->search(ele);
        if (buck->flag != SET_FLAG_FULL)
            return;
        buck->flag = SET_FLAG_DEAD;
        while (
            buck->flag == SET_FLAG_DEAD &&
            next(buck)->flag == SET_FLAG_EMPTY)
        {
            buck = prev(buck);
        }
    }
    T *get(const T &ele) const
    {
        Bucket<T> *buck = this->search(ele);
        if (buck->flag == SET_FLAG_FULL)
        {
            return &buck->val;
        }
        else
        {
            return nullptr;
        }
    }
    bool contains(const T &ele)
    {
        return this->search(ele)->flag == SET_FLAG_FULL;
    }
    T &operator[](const T &ele)
    {
        return this->search(ele)->val;
    }
    T *iter(int &idx) const
    {
        do
        {
            idx++;
            if (idx >= (int)this->cap)
                return nullptr;
        } while (this->buffer[idx].flag != SET_FLAG_FULL);
        return &this->buffer[idx].val;
    }
};

#endif