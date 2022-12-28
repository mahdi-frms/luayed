#ifndef LUADEF_h
#define LUADEF_h

#include <vector>
#include <string>

template <typename T>
using vector = std::vector<T>;

typedef std::string string;

typedef unsigned char lbyte;
typedef double lnumber;

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

    bool truth();
    const char *as_string();
};

struct Hook
{
    bool is_detached;
    LuaValue val;
    LuaValue *original;
};

bool operator==(const LuaValue &v1, const LuaValue &v2);
std::ostream &operator<<(std::ostream &strm, const LuaValue &v);
std::ostream &operator<<(std::ostream &strm, const LuaType &t);

#endif