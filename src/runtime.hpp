#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <set>
#include <map>
#include <vector>

#define lstrnull NULL

class LuaValue;
class Lua;
typedef size_t (*LuaCppFunction)(Lua *);
struct LuaFunction;

template <typename T>
using vector = std::vector<T>;

typedef std::string string;

typedef unsigned char lbyte;
typedef double lnumber;

enum Instruction
{
    IAdd = 0x10,
    ISub = 0x11,
    IMult = 0x12,
    IFlrDiv = 0x13,
    IFltDiv = 0x14,
    IMod = 0x15,
    IPow = 0x16,
    IConcat = 0x17,
    IBOr = 0x18,
    IBAnd = 0x19,
    IBXor = 0x1a,
    ISHR = 0x1b,
    ISHL = 0x1c,

    ILength = 0x20,
    INegate = 0x21,
    INot = 0x22,
    IBNot = 0x23,

    IEq = 0x30,
    INe = 0x31,
    IGe = 0x32,
    IGt = 0x33,
    ILe = 0x34,
    ILt = 0x35,

    ITGet = 0x40,
    ITSet = 0x41,
    ITNew = 0x42,
    IGGet = 0x43,
    IGSet = 0x44,
    INil = 0x45,
    ITrue = 0x46,
    IFalse = 0x47,

    ITList = 0xc0,
    IRet = 0xc2,

    IFVargs = 0x54,

    ICall = 0xd0,
    IVargs = 0xd4,
    IJmp = 0xd6,
    ICjmp = 0xd8,

    INConst = 0xe0,
    ISConst = 0xe2,
    IFConst = 0xe4,

    ILocal = 0xf0,
    ILStore = 0xf2,
    IBLocal = 0xf4,
    IBLStore = 0xf6,
    IUpvalue = 0xf8,
    IUStore = 0xfa,

    IPush = 0xfc,
    IPop = 0xfe
};

struct Upvalue
{
    size_t offset;
    size_t fn_idx;
};

class Lfunction
{
public:
    size_t codelen = 0;
    size_t uplen = 0;
    size_t rolen = 0;
    size_t parlen = 0;
    size_t fidx = 0;
    size_t stack_size = 0;
    size_t upvalue_size = 0;

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

class StringInterner
{
private:
    std::set<InternString> set;

public:
    char *insert(char *lstr);
    void remove(char *lstr);
};

class Frame;

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
    size_t stack_size;
    size_t sp;
    LuaValue fn;
    Frame *prev;
    size_t retc;
    size_t vargsc;

    Lfunction *bin();
    LuaValue *stack();
    LuaValue *args();
    Hook *uptable();
    Hook *hooktable();

    LuaValue pop();
    void push(LuaValue value);
};

class Lua
{
public:
    StringInterner interner;
    vector<Lfunction *> functable;
    Frame *frame;

    LuaValue create_nil();
    LuaValue create_boolean(bool b);
    LuaValue create_number(lnumber n);
    LuaValue create_string(const char *s);
    LuaValue create_table();
    Lfunction *create_binary(
        vector<lbyte> &text,
        vector<LuaValue> &rodata,
        vector<Upvalue> &ups,
        size_t parlen,
        size_t fidx,
        size_t stack_size,
        size_t upvalue_size);
    LuaValue create_cppfn(LuaCppFunction fn);

    LuaValue clone_value(LuaValue &value);
    void destroy_value(LuaValue &value);
    void *allocate(size_t size);
    void deallocate(void *ptr);

    void fncall(size_t argc, size_t retc);
    void fnret(size_t count);
    void new_frame(size_t stack_size);
    void destroy_frame();
};

struct LuaFunction
{
    void *fn;
    bool is_lua;
};

#endif