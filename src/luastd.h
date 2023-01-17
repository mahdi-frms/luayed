#ifndef LUASTD_H
#define LUASTD_H

#include "lua.h"

namespace luastd
{
    string luavalue_to_string(Lua *lua);

    size_t print(Lua *lua);
    size_t unpack(Lua *lua);
    size_t to_string(Lua *lua);

    void libinit(Lua *lua);
}
#endif