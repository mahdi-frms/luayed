#include "test.h"
#include <lua.h>
#include <parser.h>
#include <resolve.h>
#include <generator.h>
#include <compiler.h>
#include <lstrep.h>

void print_valvec(vector<LuaValue> &stack)
{
    for (size_t i = 0; i < stack.size(); i++)
        std::cerr << stack[i] << "\n";
}

void lua_test_case(
    const char *message,
    const char *code,
    vector<LuaValue> results = {},
    vector<LuaValue> args = {})
{
    string mes = "lua : ";
    mes.append(message);
    Interpreter intp;
    LuaRuntime rt(&intp);
    Lexer lexer(code);
    Parser parser(&lexer);
    Ast ast = parser.parse();
    if (ast.root() == nullptr)
    {
        std::cerr << "Compiling test case failed: \n"
                  << parser.get_error();
        exit(1);
    }
    SemanticAnalyzer sem(ast);
    auto errs = sem.analyze();
    if (errs.size())
    {
        std::cerr << "Compiling test case failed!\n";
        exit(1);
    }
    LuaGenerator gen(&rt);
    Compiler compiler(&gen);
    fidx_t fidx = compiler.compile(ast);
    LuaValue fn = rt.create_luafn(fidx);
    rt.stack_push(fn);
    for (size_t i = 0; i < args.size(); i++)
    {
        rt.stack_push(args[i]);
    }
    rt.fncall(args.size(), results.size());
    for (size_t i = 0; i < args.size(); i++)
    {
        rt.stack_push(args[i]);
    }
    vector<LuaValue> stack;
    while (rt.stack_size())
        stack.insert(stack.begin(), rt.stack_pop());
    bool rsl = stack == results;
    test_assert(rsl, mes.c_str());
    if (!rsl)
    {
        std::cerr << "stack: \n";
        print_valvec(stack);
        std::cerr << "expected: \n";
        print_valvec(results);
    }
}

void lua_tests()
{
    lua_test_case(
        "simple return",
        "return 9",
        {
            lvnumber(9),
        });

    lua_test_case(
        "return local",
        "local a = 9 return a",
        {
            lvnumber(9),
        });

    lua_test_case(
        "simple math",
        "local a = 1 + 8 return a",
        {
            lvnumber(9),
        });
}