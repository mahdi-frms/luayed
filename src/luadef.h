#ifndef LUADEF_h
#define LUADEF_h

#include <vector>
#include <string>
#include <iostream>

template <typename T>
using vector = std::vector<T>;

typedef std::string string;

typedef unsigned char lbyte;
typedef double lnumber;

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

    bool truth()
    {
        return this->kind != LuaType::LVNil && (this->kind != LuaType::LVBool || this->data.b);
    }
    template <typename T>
    T as()
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

bool operator==(const LuaValue &v1, const LuaValue &v2);
bool operator!=(const LuaValue &v1, const LuaValue &v2);

#endif