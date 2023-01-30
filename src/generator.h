#ifndef GENERATOR_h
#define GENERATOR_h

#include "runtime.h"
#include "luabin.h"
#include <map>

struct FuncTest
{
    FuncTest *prev;
    vector<lbyte> text;
    vector<Upvalue> upvalues;
    vector<size_t> debug;
    size_t ccount;
    size_t hookmax;
    size_t parcount;
    fidx_t fidx;
};

class BaseGenerator : public IGenerator
{
protected:
    std::map<fidx_t, FuncTest *> funcs;
    FuncTest *current;
    const char *message;
    FuncTest *test;
    fidx_t fidx_counter = 1;

public:
    BaseGenerator(const char *message) : current(nullptr), message(message), test(nullptr)
    {
    }
    void emit(Opcode opcode)
    {
        for (size_t i = 0; i < opcode.count; i++)
            this->current->text.push_back(opcode.bytes[i]);
    }
    size_t len()
    {
        return this->current->text.size();
    }
    void debug_info(size_t line)
    {
        line++;
        while (this->current->debug.size() <= this->current->text.size())
            this->current->debug.push_back(0);
        this->current->debug.back() = line;
    }
    void seti(size_t idx, lbyte b)
    {
        this->current->text[idx] = b;
    }
    size_t const_number(lnumber num)
    {
        return this->current->ccount++;
    }
    size_t const_string(const char *str)
    {
        return this->current->ccount++;
    }
    fidx_t pushf()
    {
        FuncTest *fnt = new FuncTest();
        fidx_t fidx = this->fidx_counter++;
        fnt->ccount = 0;
        fnt->prev = nullptr;
        fnt->hookmax = 0;
        fnt->parcount = 0;
        fnt->prev = this->current;
        fnt->fidx = fidx;
        this->current = fnt;
        return fidx;
    }
    void popf()
    {
        this->funcs[this->current->fidx] = this->current;
        this->current = this->current->prev;
    }
    size_t upval(fidx_t fidx, size_t offset, size_t hidx)
    {
        size_t idx = this->current->upvalues.size();
        this->current->upvalues.push_back(Upvalue(fidx, offset, hidx));
        return idx;
    }
    void meta_parcount(size_t parcount)
    {
        this->current->parcount = parcount;
    }
    void meta_hookmax(size_t hookmax)
    {
        this->current->hookmax = hookmax;
    }

    void meta_chunkname(const char *chunkname)
    {
    }
    ~BaseGenerator()
    {
        for (auto it = this->funcs.begin(); it != this->funcs.end(); it++)
            delete it->second;
    }
};

class LuaGenerator final : public IGenerator
{
private:
    LuaRuntime *rt;
    GenFunction *gfn;
    size_t add_const(LuaValue value);

public:
    LuaGenerator(LuaRuntime *rt);

    void emit(Opcode opcode);
    size_t len();
    void seti(size_t idx, lbyte b);

    size_t const_number(lnumber num);
    size_t const_string(const char *str);
    void debug_info(size_t line);

    fidx_t pushf();
    void popf();

    size_t upval(fidx_t fidx, size_t offset, size_t hidx);

    void meta_parcount(size_t parcount);
    void meta_hookmax(size_t hookmax);
    void meta_chunkname(const char *chunkname);
};

#endif