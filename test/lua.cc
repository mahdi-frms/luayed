#include "test.h"
#include <lua.h>
#include <parser.h>
#include <resolve.h>
#include <generator.h>
#include <compiler.h>
#include <lstrep.h>

LuaValue lua_test_compile(const char *code, LuaRuntime &rt, vector<lbyte> &bin)
{
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
    vector<LError> errs = sem.analyze();
    if (errs.size())
    {
        std::cerr << "Compiling test case failed: \n";
        for (size_t i = 0; i < errs.size(); i++)
            std::cerr << errs[i];
        exit(1);
    }
    LuaGenerator gen(&rt);
    Compiler compiler(&gen);
    fidx_t fidx = compiler.compile(ast);
    LuaValue fn = rt.create_luafn(fidx);

    size_t bytecodelen = rt.bin(fidx)->codelen;
    bin.resize(bytecodelen);
    std::copy(
        rt.bin(fidx)->text(),
        rt.bin(fidx)->text() + bytecodelen,
        bin.begin());

    return fn;
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
    vector<lbyte> bytecode;
    LuaValue fn = lua_test_compile(code, rt, bytecode);
    rt.stack_push(fn);
    pipe(&rt, args);
    rt.fncall(args.size(), results.size() + 1);
    vector<LuaValue> stack = drain(&rt);
    bool rsl = stack == results;
    test_assert(rsl, mes.c_str());
    if (!rsl)
    {
        std::cerr << "stack:\n"
                  << stack
                  << "expected:\n"
                  << results << "\n"

                  << "binary:\n"
                  << bytecode
                  << "\n";
    }
}

void lua_tests()
{
    lua_test_case("nothing", "return 7", {lvnumber(7)});

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

    lua_test_case(
        "more complex math",
        "local a = -(-(1 + (10 - 2))) + 10 return a",
        {
            lvnumber(19),
        });

    lua_test_case(
        "simple control",

        "local a = 8\n"
        "local b = 1\n"
        "if a == 6 then b = b - 1 else b = b + 1 end\n"
        "return b",
        {
            lvnumber(2),
        });

    lua_test_case(
        "return explist",

        "return true, 7",
        {
            lvbool(true),
            lvnumber(7),
        });

    lua_test_case(
        "varlist declaration",

        "local i,j = 12,10 return i,j",
        {
            lvnumber(12),
            lvnumber(10),
        });

    lua_test_case(
        "group assignment",

        "local i,j i,j = 12,10 return i,j",
        {
            lvnumber(12),
            lvnumber(10),
        });

    lua_test_case(
        "function call",

        "local function v() return 8 end\n"
        "local u = v()\n"
        "return u",
        {
            lvnumber(8),
        });

    lua_test_case(
        "return call",

        "local function v() return 8 end\n"
        "return v()",
        {
            lvnumber(8),
        });

    lua_test_case(
        "return function",

        "local function v() return function(b) return not b end end\n"
        "return v()(false)",
        {
            lvbool(true),
        });

    lua_test_case(
        "while loop",

        "local t,v = 10,0 while t > 0 do t = t - 1 v = v + 5 end return v",
        {
            lvnumber(50),
        });

    lua_test_case(
        "repeat loop",

        "local t,v = 10,0 repeat t = t - 1 v = v + 5 until t == 0 return v",
        {
            lvnumber(50),
        });
}