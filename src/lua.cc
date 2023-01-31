#include "lua.h"
#include "luastate.h"

Lua *Lua::create(LuaConfig conf)
{
    return new LuaState(conf);
}

void Lua::destroy()
{
    delete (LuaState *)this;
}
Lua::~Lua()
{
}