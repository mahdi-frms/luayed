#ifndef TEST_H
#define TEST_H

#include <luadef.h>

LuaValue lvnil();
LuaValue lvbool(bool b);
LuaValue lvnumber(lnumber n);
LuaValue lvstring(const char *s);
LuaValue lvtable();

void test_assert(bool result, const char *message);

#endif