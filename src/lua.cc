#include "lua.h"
#include "luastate.h"

Lua *Lua::create(LuaConfig conf)
{
    return new LuaState(conf);
}