#include "parser.h"
#include "resolve.h"
#include "generator.h"
#include "compiler.h"
#include "runtime.h"
#include "interpreter.h"
#include "luastate.h"
#include "lstrep.h"
#include "luastd.h"
#include "reader.h"

LuaState::LuaState(LuaConfig conf) : runtime(&this->interpreter)
{
    this->runtime.set_lua_interface(this);
    this->interpreter.config_error_metadata(conf.error_metadata);
    if (conf.load_stdlib)
        luastd::libinit(this);
}
int LuaState::compile(const char *lua_code, string &errors, const char *chunkname)
{
    errors.clear();
    StringSourceReader reader(lua_code);
    Lexer lexer(&reader);
    Parser parser(&lexer);
    Ast ast = parser.parse();
    if (ast.root() == nullptr)
    {
        errors.append(to_string(parser.get_error()));
        return LUA_COMPILE_RESULT_FAILED;
    }
    Resolver sem(ast, lua_code);
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
    compiler.compile(ast, lua_code, chunkname);
    this->runtime.push_compiled_bin();
    return LUA_COMPILE_RESULT_OK;
}
void LuaState::push_number(lnumber num)
{
    this->runtime.stack_push(this->runtime.create_number(num));
}
void LuaState::push_boolean(bool b)
{
    this->runtime.stack_push(this->runtime.create_boolean(b));
}
void LuaState::insert(size_t index)
{
    LuaValue v = this->runtime.stack_pop();
    this->runtime.stack_push(this->runtime.create_nil());
    for (size_t i = this->runtime.stack_size() - 1; i > index; i--)
    {
        this->runtime.stack_write(i, this->runtime.stack_read(i - 1));
    }
    this->runtime.stack_write(index, v);
}
void LuaState::fetch_local(int idx)
{
    if (idx >= 0)
        this->runtime.stack_push(this->runtime.stack_read(idx));
    else
        this->runtime.stack_push(this->runtime.stack_back_read(-idx));
}
void LuaState::store_local(int idx)
{
    if (idx >= 0)
        this->runtime.stack_write(idx, this->runtime.stack_pop());
    else
        this->runtime.stack_back_write(-idx, this->runtime.stack_pop());
}
void LuaState::push_cppfn(LuaCppFunction cppfn)
{
    LuaValue fn = this->runtime.create_cppfn((LuaRTCppFunction)cppfn);
    this->runtime.stack_push(fn);
}
void LuaState::call(size_t arg_count, size_t return_count)
{
    this->runtime.fncall(arg_count, return_count == LUA_MULTRES ? 0 : return_count + 1);
}
int LuaState::kind()
{
    LuaValue value = this->runtime.stack_pop();
    this->runtime.stack_push(value);
    return value.kind;
}

void LuaState::pop()
{
    this->runtime.stack_pop();
}
lnumber LuaState::pop_number()
{
    return this->runtime.stack_pop().data.n;
}
bool LuaState::pop_boolean()
{
    return this->runtime.stack_pop().data.b;
}
const char *LuaState::peek_string()
{
    return this->runtime.stack_back_read(1).as<const char *>();
}
bool LuaState::has_error()
{
    return this->runtime.error_raised();
}
void LuaState::push_error()
{
    LuaValue e = this->runtime.get_error();
    this->runtime.remove_error();
    this->runtime.stack_push(e);
}
void LuaState::pop_error()
{
    LuaValue e = this->runtime.stack_pop();
    this->runtime.set_error(e);
}

void LuaState::push_string(const char *str)
{
    LuaValue s = this->runtime.create_string(str);
    this->runtime.stack_push(s);
}
void LuaState::set_global(const char *key)
{
    // todo : handle error
    LuaValue value = this->runtime.stack_pop();
    LuaValue gkey = this->runtime.create_string(key);
    this->runtime.table_set(this->runtime.table_global(), gkey, value);
}
size_t LuaState::top()
{
    return this->runtime.stack_size();
}
void LuaState::push_nil()
{
    this->runtime.stack_push(this->runtime.create_nil());
}
void LuaState::set_table()
{
    LuaValue value = this->runtime.stack_pop();
    LuaValue key = this->runtime.stack_pop();
    LuaValue table = this->runtime.stack_pop();
    this->runtime.table_set(table, key, value);
    this->runtime.stack_push(table);
}
void LuaState::get_table()
{
    LuaValue key = this->runtime.stack_pop();
    LuaValue table = this->runtime.stack_pop();
    LuaValue value = this->runtime.table_get(table, key);
    this->runtime.stack_push(value);
}