#ifndef VIRTUALS_H
#define VIRTUALS_H

#include "luadef.h"
#include "luabin.h"
#include "token.h"
#include "lerror.h"

class IGenerator
{
public:
    virtual void emit(Opcode opcode) = 0;
    virtual size_t len() = 0;
    virtual void seti(size_t idx, lbyte b) = 0;

    virtual size_t const_number(lnumber num) = 0;
    virtual size_t const_string(const char *str) = 0;

    virtual fidx_t pushf() = 0;
    virtual void popf() = 0;

    virtual size_t upval(fidx_t fidx, size_t offset, size_t hidx) = 0;

    virtual void meta_parcount(size_t parcount) = 0;
    virtual void meta_hookmax(size_t hookmax) = 0;
};

class ILexer
{
public:
    virtual Lerror get_error() = 0;
    virtual Token next() = 0;
};

class IRuntime
{
public:
    virtual LuaValue create_nil() = 0;
    virtual LuaValue create_boolean(bool b) = 0;
    virtual LuaValue create_number(lnumber n) = 0;
    virtual LuaValue create_string(const char *s) = 0;
    virtual LuaValue create_string(const char *s1, const char *s2) = 0;
    virtual LuaValue create_table() = 0;
    virtual LuaValue create_luafn(fidx_t fidx) = 0;

    virtual void fncall(size_t argc, size_t retc) = 0;
    virtual void set_error(LuaValue value) = 0;
    virtual LuaValue get_error() = 0;

    virtual LuaValue stack_pop() = 0;
    virtual void stack_push(LuaValue value) = 0;
    virtual LuaValue stack_read(size_t idx) = 0;
    virtual void stack_write(size_t idx, LuaValue value) = 0;
    virtual LuaValue stack_back_read(size_t idx) = 0;
    virtual void stack_back_write(size_t idx, LuaValue value) = 0;
    virtual void hookpush() = 0;
    virtual void hookpop() = 0;
    virtual LuaValue arg(size_t idx) = 0;
    virtual size_t load_ip() = 0;
    virtual void save_ip(size_t sp) = 0;
    virtual Hook *upvalue(size_t idx) = 0;
    virtual LuaValue rodata(size_t idx) = 0;
    virtual lbyte *text() = 0;
};

class IInterpreter
{
public:
    virtual size_t run(IRuntime *rt) = 0;
    virtual size_t run(IRuntime *rt, Opcode op) = 0;
};

class IAllocator
{
public:
    virtual void *allocate(size_t size);
    virtual void deallocate(void *ptr);
};

#endif