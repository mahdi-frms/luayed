#ifndef GENERATOR_h
#define GENERATOR_h

#include "runtime.h"
#include "luabin.h"

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