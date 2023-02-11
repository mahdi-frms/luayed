#ifndef GENERATOR_h
#define GENERATOR_h

#include "runtime.h"
#include "luabin.h"

namespace luayed
{
    struct FuncTest
    {
        FuncTest *prev;
        vector<lbyte> text;
        vector<Upvalue> upvalues;
        vector<size_t> debug;
        vector<string> constants;
        size_t hookmax;
        size_t parcount;
        fidx_t fidx;
    };

    class BaseGenerator : public IGenerator
    {
    protected:
        vector<FuncTest *> funcs;
        FuncTest *current;
        FuncTest *test;
        fidx_t fidx_counter = 1;

    public:
        BaseGenerator();
        void emit(Bytecode opcode);
        size_t len();
        void debug_info(size_t line);
        void seti(size_t idx, lbyte b);
        size_t const_number(lnumber num);
        size_t const_string(const char *str);
        fidx_t pushf();
        void popf();
        size_t upval(Upvalue upvalue);
        void meta_parcount(size_t parcount);
        void meta_hookmax(size_t hookmax);
        void meta_chunkname(const char *chunkname);
        ~BaseGenerator();
    };

    class LuaGenerator final : public IGenerator
    {
    private:
        LuaRuntime *rt;
        GenFunction *gfn;
        size_t add_const(LuaValue value);

    public:
        LuaGenerator(LuaRuntime *rt);

        void emit(Bytecode opcode);
        size_t len();
        void seti(size_t idx, lbyte b);

        size_t const_number(lnumber num);
        size_t const_string(const char *str);
        void debug_info(size_t line);

        fidx_t pushf();
        void popf();

        size_t upval(Upvalue upvalue);

        void meta_parcount(size_t parcount);
        void meta_hookmax(size_t hookmax);
        void meta_chunkname(const char *chunkname);
    };
};

#endif