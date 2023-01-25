#include "lua.h"
#include "parser.h"
#include "resolve.h"
#include "generator.h"
#include "compiler.h"
#include "lstrep.h"
#include "luastd.h"

Lua::Lua() : runtime(&this->interpreter)
{
    this->runtime.set_lua_interface(this);
    luastd::libinit(this);
}
int Lua::compile(const char *lua_code, string &errors, const char *chunkname)
{
    Lexer lexer(lua_code);
    Parser parser(&lexer);
    Ast ast = parser.parse();
    if (ast.root() == nullptr)
    {
        errors.append(to_string(parser.get_error()));
        return LUA_COMPILE_RESULT_FAILED;
    }
    Resolver sem(ast);
    vector<Lerror> errs = sem.analyze();
    if (errs.size())
    {
        errors.append(to_string(errs[0]));
        for (size_t i = 1; i < errs.size(); i++)
        {
            errors.push_back('\n');
            errors.append(to_string(errs[i]));
        }
        return LUA_COMPILE_RESULT_FAILED;
    }
    LuaGenerator gen(&this->runtime);
    Compiler compiler(&gen);
    fidx_t fidx = compiler.compile(ast, chunkname);
    LuaValue fn = this->runtime.create_luafn(fidx);
    this->runtime.stack_push(fn);
    return LUA_COMPILE_RESULT_OK;
}
void Lua::push_number(lnumber num)
{
    this->runtime.stack_push(this->runtime.create_number(num));
}
void Lua::push_boolean(bool b)
{
    this->runtime.stack_push(this->runtime.create_boolean(b));
}
void Lua::insert(size_t index)
{
    LuaValue v = this->runtime.stack_pop();
    this->runtime.stack_push(this->runtime.create_nil());
    for (size_t i = this->runtime.stack_size() - 1; i > index; i--)
    {
        this->runtime.stack_write(i, this->runtime.stack_read(i - 1));
    }
    this->runtime.stack_write(index, v);
}
void Lua::fetch_local(int idx)
{
    if (idx >= 0)
        this->runtime.stack_push(this->runtime.stack_read(idx));
    else
        this->runtime.stack_push(this->runtime.stack_back_read(-idx));
}
void Lua::store_local(int idx)
{
    if (idx >= 0)
        this->runtime.stack_write(idx, this->runtime.stack_pop());
    else
        this->runtime.stack_back_write(-idx, this->runtime.stack_pop());
}
void Lua::push_cppfn(LuaCppFunction cppfn)
{
    LuaValue fn = this->runtime.create_cppfn((LuaRTCppFunction)cppfn);
    this->runtime.stack_push(fn);
}
void Lua::call(size_t arg_count, size_t return_count)
{
    this->runtime.fncall(arg_count, return_count);
}
int Lua::kind()
{
    LuaValue value = this->runtime.stack_pop();
    this->runtime.stack_push(value);
    return value.kind;
}

void Lua::pop()
{
    this->runtime.stack_pop();
}
lnumber Lua::pop_number()
{
    return this->runtime.stack_pop().data.n;
}
bool Lua::pop_boolean()
{
    return this->runtime.stack_pop().data.b;
}
const char *Lua::peek_string()
{
    return this->runtime.stack_back_read(1).as<const char *>();
}
bool Lua::has_error()
{
    return this->runtime.error_raised();
}
void Lua::push_error()
{
    LuaValue e = this->runtime.get_error();
    this->runtime.remove_error();
    this->runtime.stack_push(e);
}
void Lua::pop_error()
{
    LuaValue e = this->runtime.stack_pop();
    this->runtime.set_error(e);
}

void Lua::push_string(const char *str)
{
    LuaValue s = this->runtime.create_string(str);
    this->runtime.stack_push(s);
}
void Lua::set_global()
{
    // todo : handle error
    LuaValue value = this->runtime.stack_pop();
    LuaValue key = this->runtime.stack_pop();
    this->runtime.table_set(this->runtime.table_global(), key, value);
}
size_t Lua::top()
{
    return this->runtime.stack_size();
}
void Lua::push_nil()
{
    this->runtime.stack_push(this->runtime.create_nil());
}
void Lua::set_table()
{
    LuaValue value = this->runtime.stack_pop();
    LuaValue key = this->runtime.stack_pop();
    LuaValue table = this->runtime.stack_pop();
    this->runtime.table_set(table, key, value);
    this->runtime.stack_push(table);
}
void Lua::get_table()
{
    LuaValue key = this->runtime.stack_pop();
    LuaValue table = this->runtime.stack_pop();
    LuaValue value = this->runtime.table_get(table, key);
    this->runtime.stack_push(value);
}