#include "lua.hpp"
#include "parser.hpp"
#include "resolve.hpp"
#include "generator.hpp"
#include "compiler.hpp"

void Lua::compile(const char *lua_code)
{
    Lexer lexer(lua_code);
    Parser parser((ILexer *)&lexer);
    Ast ast = parser.parse();
    if (ast.root() == nullptr)
    {
        // todo : should return error
        return;
    }
    size_t bin_fidx = this->runtime.functable.size();
    SemanticAnalyzer sem(ast, bin_fidx);
    auto errs = sem.analyze();
    if (errs.size())
    {
        // todo : should return error
        return;
    }
    LuaGenerator gen(&this->runtime);
    Compiler compiler((IGenerator *)&gen);
    compiler.compile(ast);
    Lfunction *bin = this->runtime.functable[bin_fidx];
    LuaValue fn = this->runtime.create_luafn(bin);
    this->runtime.stack_push(fn);
}
void Lua::push_cppfn(LuaCppFunction cppfn)
{
    LuaValue fn = this->runtime.create_cppfn(cppfn);
    this->runtime.stack_push(fn);
}
void Lua::call(size_t arg_count, size_t return_count)
{
    this->runtime.fncall(arg_count, return_count + 1);
}