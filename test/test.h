#ifndef TEST_H
#define TEST_H

#include <runtime.h>

vector<LuaValue> drain(LuaRuntime *rt);
void pipe(LuaRuntime *rt, vector<LuaValue> values);

LuaValue lvnil();
LuaValue lvbool(bool b);
LuaValue lvnumber(lnumber n);
LuaValue lvstring(const char *s);
LuaValue lvtable();
LuaValue lvclone(LuaRuntime *rt, const LuaValue &v);

void test_assert(bool result, const char *message);

#endif