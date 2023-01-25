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
    virtual void debug_info(size_t line) = 0;

    virtual fidx_t pushf() = 0;
    virtual void popf() = 0;

    virtual size_t upval(fidx_t fidx, size_t offset, size_t hidx) = 0;

    virtual void meta_parcount(size_t parcount) = 0;
    virtual void meta_hookmax(size_t hookmax) = 0;
    virtual void meta_chunkname(const char *chunkname) = 0;
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
    virtual LuaValue create_string(lnumber n) = 0;
    virtual LuaValue create_string(const char *s) = 0;
    virtual LuaValue create_string(const char *s1, const char *s2) = 0;
    virtual LuaValue create_table() = 0;
    virtual LuaValue create_luafn(fidx_t fidx) = 0;

    virtual void table_set(LuaValue t, LuaValue k, LuaValue v) = 0;
    virtual LuaValue table_get(LuaValue t, LuaValue k) = 0;
    virtual LuaValue table_global() = 0;

    virtual void fncall(size_t argc, size_t retc) = 0;
    virtual void set_error(LuaValue value) = 0;
    virtual LuaValue get_error() = 0;
    virtual bool error_raised() = 0;
    virtual bool error_metadata() = 0;
    virtual void error_metadata(bool md) = 0;
    virtual size_t extras() = 0;
    virtual void extras(size_t count) = 0;
    virtual size_t argcount() = 0;

    virtual LuaValue stack_pop() = 0;
    virtual void stack_push(LuaValue value) = 0;
    virtual LuaValue stack_read(size_t idx) = 0;
    virtual void stack_write(size_t idx, LuaValue value) = 0;
    virtual LuaValue stack_back_read(size_t idx) = 0;
    virtual void stack_back_write(size_t idx, LuaValue value) = 0;
    virtual void hookpush() = 0;
    virtual void hookpop() = 0;
    virtual LuaValue arg(size_t idx) = 0;
    virtual Hook *upvalue(size_t idx) = 0;
    virtual LuaValue rodata(size_t idx) = 0;
    virtual lbyte *text() = 0;
    virtual uint16_t *dbgmd() = 0;
    virtual LuaValue chunkname() = 0;
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
    virtual void *allocate_raw(size_t size) = 0;
    virtual void deallocate_raw(void *ptr) = 0;
};

class IGarbageCollector
{
public:
    virtual void reference(void *ptr) = 0;
};

#endif