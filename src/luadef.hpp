#ifndef LUADEF_HPP
#define LUADEF_HPP

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

#endif