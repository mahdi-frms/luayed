#include "lua.h"
#include "parser.h"
#include "resolve.h"
#include "generator.h"
#include "compiler.h"

Lua::Lua() : runtime(&this->interpreter)
{
    this->runtime.set_lua_interface(this);
}

void Lua::compile(const char *lua_code)
{
    Lexer lexer(lua_code);
    Parser parser(&lexer);
    Ast ast = parser.parse();
    if (ast.root() == nullptr)
    {
        // todo : should return error
        return;
    }
    SemanticAnalyzer sem(ast);
    auto errs = sem.analyze();
    if (errs.size())
    {
        // todo : should return error
        return;
    }
    LuaGenerator gen(&this->runtime);
    Compiler compiler(&gen);
    fidx_t fidx = compiler.compile(ast);
    LuaValue fn = this->runtime.create_luafn(fidx);
    this->runtime.stack_push(fn);
}
void Lua::push_cppfn(LuaCppFunction cppfn)
{
    LuaValue fn = this->runtime.create_cppfn((LuaRTCppFunction)cppfn);
    this->runtime.stack_push(fn);
}
void Lua::call(size_t arg_count, size_t return_count)
{
    this->runtime.fncall(arg_count, return_count + 1);
}
lnumber Lua::pop_number()
{
    LuaValue value = this->runtime.stack_pop();
    lnumber num = value.data.n;
    return num;
}