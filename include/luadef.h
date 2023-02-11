#ifndef luadef_h
#define luadef_h

#include <vector>
#include <string>
#include <iostream>
#include <stddef.h>

typedef double lnumber;
typedef std::string string;

template <typename T>
using vector = std::vector<T>;

typedef unsigned char lbyte;
extern std::ostream &dbg;
typedef size_t fidx_t;

enum LuaType
{
    LVNil = 0,
    LVBool = 1,
    LVNumber = 2,
    LVString = 3,
    LVTable = 4,
    LVFunction = 5,
};

#define is_obj(V) ((V).kind > 2)

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

    LuaValue();

    bool truth()
    {
        return this->kind != LuaType::LVNil && (this->kind != LuaType::LVBool || this->data.b);
    }
    template <typename T>
    T as() const
    {
        return ((T)this->data.ptr);
    }
};

struct Hook
{
    bool is_detached;
    LuaValue *original;
    LuaValue val;
};

struct Fnresult
{
    enum
    {
        Call,
        Ret,
        Error,
        Fail,
    } kind;

    size_t argc;
    size_t retc;
};

bool operator==(const LuaValue &v1, const LuaValue &v2);
bool operator!=(const LuaValue &v1, const LuaValue &v2);

void crash(string message);

#endif