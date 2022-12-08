#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "runtime.hpp"

struct Opcode
{
    lbyte count;
    lbyte bytes[5];

    Opcode(lbyte op, size_t idx);
    Opcode(lbyte op, size_t idx1, size_t idx2);
    Opcode(lbyte op);
};

class IGenerator
{
public:
    virtual void emit(Opcode opcode) = 0;
    virtual size_t len() = 0;
    virtual void seti(size_t idx, lbyte b) = 0;

    virtual size_t const_number(lnumber num) = 0;
    virtual size_t const_string(const char *str) = 0;

    virtual void pushf(fidx_t fidx) = 0;
    virtual void popf() = 0;

    virtual size_t upval(size_t idx, size_t offset) = 0;
};

class LuaGenerator : IGenerator
{
private:
    GenFunction *gfn;
    Lua *rt;
    size_t add_const(LuaValue value);

public:
    LuaGenerator(Lua *rt);

    void emit(Opcode opcode);
    size_t len();
    void seti(size_t idx, lbyte b);

    size_t const_number(lnumber num);
    size_t const_string(const char *str);

    void pushf(fidx_t fidx);
    void popf();
};

#endif