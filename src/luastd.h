#ifndef LUASTD_H
#define LUASTD_H

#include "lua.h"

string readfile(const char *path);

namespace luastd
{
    string luavalue_to_string(Lua *lua);

    size_t print(Lua *lua);
    size_t unpack(Lua *lua);
    size_t tostring(Lua *lua);

    void libinit(Lua *lua);
    void liblua_init(Lua *lua);
    void libcpp_init(Lua *lua);
}
#endif