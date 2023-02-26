#ifndef luadef_h
#define luadef_h

#include <vector>
#include <string>
#include <iostream>
#include <stddef.h>

#define is_obj(V) ((V).kind > 2)

typedef std::string string;

template <typename T>
using vector = std::vector<T>;

extern std::ostream &dbg;

namespace luayed
{
    typedef double lnumber;

    typedef unsigned char lbyte;
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
            Tail,
            Ret,
            Error,
            Fail,
        } kind;

        size_t argc;
        size_t retc;
    };

    void crash(string message);

    bool operator==(const LuaValue &v1, const LuaValue &v2);
    bool operator!=(const LuaValue &v1, const LuaValue &v2);

};

#endif