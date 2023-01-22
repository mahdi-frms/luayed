#ifndef RUNTIME_h
#define RUNTIME_h

#include "luabin.h"
#include "virtuals.h"
#include "set.h"

#define STACK_BUFFER_SIZE 1024 * 1024

class LuaValue;
class LuaRuntime;
class Frame;
class GenFunction;
typedef size_t (*LuaRTCppFunction)(void *);
struct LuaFunction;
struct gc_header;

enum AllocType
{
    ATHook,
    ATString,
    ATTable,
    ATFunction,
    ATBinary,
    ATDummy,
};

struct gc_header
{
    gc_header *next;
    gc_header *prev;
    gc_header *scan;
    bool marked;
    AllocType alloc_type;
};

class Lfunction
{
public:
    size_t codelen = 0;
    size_t uplen = 0;
    size_t rolen = 0;
    size_t inlen = 0;
    size_t hookmax = 0;
    size_t parcount = 0;
    fidx_t fidx = 0;

    lbyte *text();
    Upvalue *ups();
    LuaValue *rodata();
    fidx_t *innerfns();

    ~Lfunction();
    Lfunction &operator=(const Lfunction &other) = delete;
    Lfunction(const Lfunction &other) = delete;

    Lfunction(Lfunction &&other) = default;
    Lfunction &operator=(Lfunction &&other) = default;
    Lfunction() = default;
};

struct Frame
{
    size_t sp;
    LuaValue fn;
    LuaValue error;
    Frame *prev;
    size_t hookptr;
    bool has_error;

    // number of args this frame is supposed to return
    size_t exp_count;
    // number of extra args supplied to this function
    size_t vargs_count;
    // number of values returned to this function after an expect-free call
    size_t ret_count;

    bool is_Lua();
    Lfunction *bin();
    size_t parcount();
    size_t hookmax();
    LuaValue *stack();
    LuaValue *vargs();
    size_t vargcount();
    Hook **uptable();
    Hook **hooktable();
    size_t stack_address(size_t idx);
};
struct GenFunction
{
    GenFunction *prev;
    vector<lbyte> text;
    vector<LuaValue> rodata;
    vector<Upvalue> upvalues;
    vector<fidx_t> innerfns;
    fidx_t fidx;
    size_t parcount;
    size_t hookmax;
};
struct lstr_t
{
    hash_t hash;
    size_t len;

    const char *cstr()
    {
        return (const char *)(this + 1);
    }
};

typedef lstr_t *lstr_p;

class LuaRuntime : public IRuntime, public IAllocator
{
private:
    Set<lstr_p> lstrset;
    Frame *frame;
    void *stack_buffer;
    IInterpreter *interpreter;
    Lfunction **functable;
    size_t func_count;
    void *lua_interface = nullptr;
    LuaValue global;
    bool test_mode = false;
    gc_header *heap_head;
    gc_header *heap_tail;

    void new_frame();
    void copy_values(Frame *fsrc, Frame *fdest, size_t count);
    void push_nils(Frame *fsrc, size_t count);
    LuaValue concat(LuaValue v1, LuaValue v2);
    LuaValue lua_type_to_string(LuaType t);

    LuaValue *stack();
    LuaValue *args();
    Hook **hooktable();
    Hook **uptable();
    Lfunction *bin();
    LuaValue error_to_string(Lerror error);

    void *allocate_raw(size_t size);
    void *allocate(size_t size, AllocType at);
    void deallocate(void *ptr);
    void heap_init();
    void heap_insert(gc_header *node, gc_header *prev, gc_header *next);
    void heap_remove(gc_header *node);

public:
    LuaRuntime(IInterpreter *interpreter);
    ~LuaRuntime();
    void set_lua_interface(void *lua_interface);
    void deallocate_obj(gc_header *hdr);

    LuaValue create_nil();
    LuaValue create_boolean(bool b);
    LuaValue create_number(lnumber n);
    LuaValue create_string(const char *s);
    LuaValue create_string(lnumber n);
    LuaValue create_string(const char *s1, const char *s2);
    LuaValue create_table();
    Lfunction *create_binary(GenFunction *gfn);
    LuaValue create_cppfn(LuaRTCppFunction fn);
    LuaValue create_luafn(fidx_t fidx);

    void table_set(LuaValue t, LuaValue k, LuaValue v);
    LuaValue table_get(LuaValue t, LuaValue k);
    LuaValue table_global();
    bool table_check(LuaValue t, LuaValue k);

    void fncall(size_t argc, size_t retc);
    void fnret(size_t count);
    void set_error(LuaValue value);
    LuaValue get_error();
    void remove_error();
    fidx_t gen_fidx();
    Lfunction *bin(size_t fidx);
    bool error_raised();
    void set_test_mode(bool mode);

    LuaValue stack_pop();
    void stack_push(LuaValue value);
    LuaValue stack_read(size_t idx);
    void stack_write(size_t idx, LuaValue value);
    LuaValue stack_back_read(size_t idx);
    void stack_back_write(size_t idx, LuaValue value);
    void hookpush();
    void hookpop();
    LuaValue arg(size_t idx);
    Hook *upvalue(size_t idx);
    LuaValue rodata(size_t idx);
    lbyte *text();
    size_t stack_size();
    size_t argcount();
    size_t extras();
    void extras(size_t count);

    Frame *topframe();
    gc_header *headers();
};

struct LuaFunction
{
    void *fn;
    bool is_lua;

    Lfunction *binary();
    LuaRTCppFunction native();
};

#endif