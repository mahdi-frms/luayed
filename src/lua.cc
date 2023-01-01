#include "lua.h"
#include "parser.h"
#include "resolve.h"
#include "generator.h"
#include "compiler.h"
#include "lstrep.h"

Lua::Lua() : runtime(&this->interpreter)
{
    this->runtime.set_lua_interface(this);
}

int Lua::compile(const char *lua_code, string &errors)
{
    Lexer lexer(lua_code);
    Parser parser(&lexer);
    Ast ast = parser.parse();
    if (ast.root() == nullptr)
    {
        errors.append(to_string(parser.get_error()));
        errors.push_back('\n');
        return LUA_COMPILE_RESULT_FAILED;
    }
    SemanticAnalyzer sem(ast);
    vector<LError> errs = sem.analyze();
    if (errs.size())
    {
        for (size_t i = 0; i < errs.size(); i++)
        {
            errors.append(to_string(errs[i]));
            errors.push_back('\n');
        }
        return LUA_COMPILE_RESULT_FAILED;
    }
    LuaGenerator gen(&this->runtime);
    Compiler compiler(&gen);
    fidx_t fidx = compiler.compile(ast);
    LuaValue fn = this->runtime.create_luafn(fidx);
    this->runtime.stack_push(fn);
    return LUA_COMPILE_RESULT_OK;
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