#ifndef VALUES_H
#define VALUES_H

#include <runtime.h>

vector<LuaValue> drain(LuaRuntime *rt);
void pipe(LuaRuntime *rt, vector<LuaValue> values);
void tabset_detroy();

LuaValue lvnil();
LuaValue lvbool(bool b);
LuaValue lvnumber(lnumber n);
LuaValue lvstring(const char *s);
LuaValue lvtable();
LuaValue lvclone(LuaRuntime *rt, const LuaValue &v);

#endif