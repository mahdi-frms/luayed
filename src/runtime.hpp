#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <set>
#include <map>
#include <vector>
#include "luabin.hpp"

#define lstrnull nullptr

class LuaValue;
class Lua;
class Frame;
class GenFunction;
typedef size_t (*LuaCppFunction)(Lua *);
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

enum LuaType
{
    LVNil = 0,
    LVBool = 1,
    LVNumber = 2,
    LVString = 3,
    LVTable = 4,
    LVFunction = 5,
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
    virtual size_t call(Lua *rt) = 0;
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
        char *s;
        LuaFunction *f;
        LuaTable *t;
    } data;
};

struct Hook
{
    LuaValue val;
    Frame *frame;
};

struct Frame
{
    size_t ss;
    size_t sp;
    LuaValue fn;
    Frame *prev;
    size_t hookptr;

    // number of args this frame is supposed to return
    size_t exp_count;
    // number of extra args supplied to this function
    size_t vargs_count;
    // number of values returned to this function after an expect-free call
    size_t ret_count;

    Lfunction *bin();
    LuaValue *stack();
    LuaValue *args();
    Hook *uptable();
    Hook *hooktable();
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
class Lua
{
public:
    StringInterner interner;
    vector<Lfunction *> functable;
    Frame *frame;
    IInterpretor *interpretor;

    LuaValue create_nil();
    LuaValue create_boolean(bool b);
    LuaValue create_number(lnumber n);
    LuaValue create_string(const char *s);
    LuaValue create_table();
    Lfunction *create_binary(GenFunction *gfn);
    LuaValue create_cppfn(LuaCppFunction fn);

    LuaValue clone_value(LuaValue &value);
    void destroy_value(LuaValue &value);
    void *allocate(size_t size);
    void deallocate(void *ptr);

    void fncall(size_t argc, size_t retc);
    void fnret(size_t count);
    void new_frame(size_t stack_size);
    void destroy_frame();

    Lfunction *bin();
    LuaValue *stack();
    LuaValue *args();
    Hook *uptable();
    Hook *hooktable();

    void copy_values(Frame *fsrc, Frame *fdest, size_t count);
    void push_nils(Frame *fsrc, size_t count);

    LuaValue stack_pop();
    void stack_push(LuaValue value);
    LuaValue stack_read(size_t idx);
    void stack_write(size_t idx, LuaValue value);
    size_t stack_ptr();
    void set_stack_ptr(size_t sp);

    size_t stack_address(size_t idx);
};

struct LuaFunction
{
    void *fn;
    bool is_lua;
};

#endif