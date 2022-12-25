#ifndef RUNTIME_h
#define RUNTIME_h

#include <set>
#include <map>
#include <vector>
#include "luabin.h"

#define lstrnull nullptr

class LuaValue;
class LuaRuntime;
class Frame;
class GenFunction;
typedef size_t (*LuaCppFunction)(LuaRuntime *);
struct LuaFunction;

class Lfunction
{
public:
    size_t codelen = 0;
    size_t uplen = 0;
    size_t rolen = 0;
    size_t hookmax = 0;
    size_t parcount = 0;
    fidx_t fidx = 0;

    lbyte *text();
    Upvalue *ups();
    LuaValue *rodata();

    string stringify();

    ~Lfunction();
    Lfunction &operator=(const Lfunction &other) = delete;
    Lfunction(const Lfunction &other) = delete;

    Lfunction(Lfunction &&other) = default;
    Lfunction &operator=(Lfunction &&other) = default;
    Lfunction() = default;
};

class LuaTable
{
private:
    std::map<LuaValue, LuaValue> map;

public:
    void set(LuaValue key, LuaValue value);
    LuaType get(LuaValue key);
};

struct InternString
{
    char *lstr;

    friend bool operator==(const InternString &l, const InternString &r);
    friend bool operator!=(const InternString &l, const InternString &r);
    friend bool operator>=(const InternString &l, const InternString &r);
    friend bool operator<=(const InternString &l, const InternString &r);
    friend bool operator>(const InternString &l, const InternString &r);
    friend bool operator<(const InternString &l, const InternString &r);
};

class IInterpretor
{
public:
    virtual size_t run(LuaRuntime *rt) = 0;
};

class StringInterner
{
private:
    std::set<InternString> set;

public:
    char *insert(char *lstr);
    void remove(char *lstr);
};

class LuaValue
{
public:
    LuaType kind;
    union
    {
        bool b;
        lnumber n;
        void *ptr;
    } data;

    bool truth();
    LuaFunction *as_function();
    const char *as_string();
};

struct Hook
{
    bool is_detached;
    LuaValue val;
    Frame *frame;
    size_t stack_idx;
};

struct Frame
{
    size_t ss;
    size_t sp;
    size_t ip;
    LuaValue fn;
    Frame *prev;
    size_t hookptr;

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
    fidx_t fidx;
    size_t parcount;
    size_t hookmax;
};
class LuaRuntime
{
private:
    StringInterner interner;
    Frame *frame;
    IInterpretor *interpretor;
    vector<Lfunction *> functable;

    void new_frame(size_t stack_size);
    void destroy_frame();
    void copy_values(Frame *fsrc, Frame *fdest, size_t count);
    void push_nils(Frame *fsrc, size_t count);

    LuaValue *stack();
    LuaValue *args();
    Hook **hooktable();
    Hook **uptable();

    void *allocate(size_t size);
    void deallocate(void *ptr);

public:
    LuaRuntime(IInterpretor *interpretor);

    LuaValue create_nil();
    LuaValue create_boolean(bool b);
    LuaValue create_number(lnumber n);
    LuaValue create_string(const char *s);
    LuaValue create_table();
    Lfunction *create_binary(GenFunction *gfn);
    LuaValue create_cppfn(LuaCppFunction fn);
    LuaValue create_luafn(Lfunction *bin);

    void fncall(size_t argc, size_t retc);
    void fnret(size_t count);

    fidx_t gen_fidx();

    void destroy_value(LuaValue &value);
    LuaValue clone_value(LuaValue &value);

    LuaValue stack_pop();
    void stack_push(LuaValue value);
    LuaValue stack_read(size_t idx);
    void stack_write(size_t idx, LuaValue value);
    LuaValue stack_back_read(size_t idx);
    void stack_back_write(size_t idx, LuaValue value);
    void hookpush();
    void hookpop();
    LuaValue hookread(Hook *hook);
    LuaValue arg(size_t idx);
    void hookwrite(Hook *hook, LuaValue value);
    size_t load_ip();
    void save_ip(size_t sp);
    Hook *upvalue(size_t idx);
    Hook *hook(size_t idx);
    Lfunction *bin();
    Lfunction *bin(size_t fidx);
};

struct LuaFunction
{
    void *fn;
    bool is_lua;

    Lfunction *binary();
    LuaCppFunction native();
};

#endif