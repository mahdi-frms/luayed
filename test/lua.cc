#include "test.h"
#include "values.h"
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
    Resolver sem(ast);
    vector<Lerror> errs = sem.analyze();
    if (errs.size())
    {
        std::cerr << "Compiling test case failed: \n";
        for (size_t i = 0; i < errs.size(); i++)
            std::cerr << errs[i] << "\n";
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
    vector<LuaValue> args = {},
    bool has_error = false,
    LuaValue error = lvnil())
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
    if (has_error)
    {
        if (rt.error_raised())
        {
            bool rsl = rt.get_error() == lvclone(&rt, error);
            test_assert(rsl, mes.c_str());
            if (!rsl)
            {
                std::cerr << "thrown: " << rt.get_error() << "\nexpected: " << error << "\n";
            }
        }
        else
        {
            test_assert(false, mes.c_str());
            std::cerr << "no error raised\n";
        }
    }
    else
    {
        if (rt.error_raised())
        {
            test_assert(false, mes.c_str());
            std::cerr << "error raised: " << rt.get_error() << "\n";
        }
        else
        {
            vector<LuaValue> stack = drain(&rt);
            for (size_t i = 0; i < results.size(); i++)
                results[i] = lvclone(&rt, results[i]);
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
    }
}

void lua_test_case_error(
    const char *message,
    const char *code,
    string error)
{
    lua_test_case(message, code, {}, {}, true, lvstring(error.c_str()));
}

void lua_tests()
{
    lua_test_case("nothing", "", {});

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

    lua_test_case(
        "varargs",

        "local function sum(...)\n"
        "    local a, b, c, d, e = ...\n"
        "    return a + b + c + d + e\n"
        "end\n"
        "return sum(1, 2, 7, 9, 6, 111)",

        {
            lvnumber(25),
        });

    lua_test_case(
        "operator precedence",

        "return (100/100 + 6) * -2 + -4 ^ 2",

        {
            lvnumber(-30),
        });

    lua_test_case(
        "strings",

        "return 'foo\\nbar' ",

        {
            lvstring("foo\nbar"),
        });

    lua_test_case(
        "strings",

        "local c1 = 'lua' == 'lua'\n"
        "local c2 = 'lu0' ~= 'luo'\n"
        "local c3 = 'lu' ..\"a\" == \"lua\"\n"
        "local c4 = 2 .. 3 == '23'\n"
        "return c1 and c2 and c3 and c4",

        {
            lvbool(true),
        });

    lua_test_case_error(
        "error: call non-function",

        "return (3)()",

        to_string(error_call_non_function(LuaType::LVNumber), true));

    lua_test_case_error(
        "error: invalid operands",

        "return 2 + true",

        to_string(error_invalid_operand(LuaType::LVBool), true));

    lua_test_case_error(
        "error: invalid comparison in another function",

        "local function cmp (a) return a > 7 end return cmp('lua')",

        to_string(error_invalid_comparison(LuaType::LVString, LuaType::LVNumber), true));

    lua_test_case(
        "passing args to main function",

        "local a,b,c,d = ... return d,c,b",

        // returned values
        {
            lvnil(),
            lvstring("test-string"),
            lvnumber(12),
        },
        // arguments
        {
            lvbool(true),            // a
            lvnumber(12),            // b
            lvstring("test-string"), // c
        });

    lua_test_case(
        "tables",

        "local function multiply(numbers)\n"
        "    local idx = 1;\n"
        "    local c = numbers[idx]\n"
        "    local r = 0\n"
        "    while c do\n"
        "        if r == 0 then\n"
        "            r = 1\n"
        "        end\n"
        "        r = r * c\n"
        "        idx = idx + 1\n"
        "        c = numbers[idx]\n"
        "    end\n"
        "    return r\n"
        "end\n"

        "return multiply {3,5,8}",

        {
            lvnumber(120),
        });

    lua_test_case(
        "globals",

        "function a() return 8 end\n"
        "return a()",

        {
            lvnumber(8),
        });

    lua_test_case(
        "methods",

        "local v = { val = 'attrib' }\n"
        "function v:get()\n"
        "    return self.val\n"
        "end\n"
        "return v:get()",

        {
            lvstring("attrib"),
        });

    lua_test_case_error(
        "error: nil index",
        "local t = {}"
        "return t[nil]",

        to_string(error_nil_index(), true));

    lua_test_case_error(
        "error: illegal indexing",
        "return (4)['key']",

        to_string(error_illegal_index(LuaType::LVNumber), true));

    lua_test_case(
        "generic for",

        "function a(s, p)\n"
        "    if p == 10\n"
        "    then\n"
        "        return nil\n"
        "    end\n"
        "    return p + 1\n"
        "end\n"
        "local sum = 0;\n"
        "for i in a, {}, 0 do\n"
        "    sum = sum + i\n"
        "end\n"
        "return sum\n",

        {
            lvnumber(55),
        });

    lua_test_case(
        "numeric for",

        "local from, to = 1, 20\n"
        "local expect = (from + to) * (to - from + 1) / 2\n"
        "local sum = 0\n"
        "for i = from, to do\n"
        "    sum = sum + i\n"
        "end\n"
        "return sum == expect\n",

        {
            lvbool(true),
        });

    lua_test_case(
        "general case 1",

        "function push(arr, val)\n"
        "    arr[#arr + 1] = val\n"
        "end\n"
        "local i = 1\n"
        "local l = {}\n"
        "while i <= 10 do\n"
        "    push(l, i)\n"
        "    i = i + 1\n"
        "end\n"
        "return #l\n",

        {
            lvnumber(10),
        });

    lua_test_case(
        "general case 2",

        "function range(f, t, s)\n"
        "    if s == nil then\n"
        "        s = 1\n"
        "    end\n"
        "    return function(_, p)\n"
        "        if p == nil then\n"
        "            return f\n"
        "        end\n"
        "        if p + s > t then\n"
        "            return nil\n"
        "        end\n"
        "        return p + s\n"
        "    end\n"
        "end\n"
        "\n"
        "function scan(iter)\n"
        "    local list = {}\n"
        "    local idx = 0\n"
        "    for i in iter do\n"
        "        idx = idx + 1\n"
        "        list[idx] = i\n"
        "    end\n"
        "    return list\n"
        "end\n"
        "\n"
        "function compare(l1, l2)\n"
        "    local i = 1\n"
        "    while l1[i] == l2[i] do\n"
        "        if (l1[i] == nil)\n"
        "        then\n"
        "            return true\n"
        "        end\n"
        "        i = i + 1\n"
        "    end\n"
        "    return false\n"
        "end\n"
        "\n"
        "return compare(scan(range(0, 50, 10)), { 0, 10, 20, 30, 40, 50 })\n"
        "\n",
        {
            lvbool(true),
        });

    lua_test_case(
        "general case 3",

        "local function iter(l)\n"
        "    return function(s, _)\n"
        "        local v = l[s.idx]\n"
        "        s.idx = s.idx + 1\n"
        "        return v\n"
        "    end, { idx = 1 }, nil\n"
        "end\n"
        "\n"
        "local function primes(to)\n"
        "    local list = {}\n"
        "    for i = 2, to do\n"
        "        local flag = true\n"
        "        for j in iter(list) do\n"
        "            if (i % j == 0) then\n"
        "                flag = false\n"
        "                break\n"
        "            end\n"
        "        end\n"
        "        if flag then\n"
        "            list[#list + 1] = i\n"
        "        end\n"
        "    end\n"
        "    return list\n"
        "end\n"
        "\n"
        "return #primes(20) == 8 -- 2,3,5,7,11,13,17,19\n"
        "\n",
        {
            lvbool(true),
        });

    lua_test_case(
        "general case 4",

        "local function sum(...)\n"
        "    local t = { ... }\n"
        "    local s = 0\n"
        "    local i = 1\n"
        "    while true do\n"
        "        local v = t[i]\n"
        "        if v == nil then\n"
        "            break\n"
        "        end\n"
        "        s = s + v\n"
        "        i = i + 1\n"
        "    end\n"
        "    return s\n"
        "end\n"
        "\n"
        "return sum(9, 8, 3, 11, 0, 4)\n"
        "\n",
        {
            lvnumber(35),
        });

    lua_test_case(
        "recursion",

        "local function fib(n)\n"
        "    if n > 2 then\n"
        "        return fib(n - 1) + fib(n - 2)\n"
        "    end\n"
        "    return 1\n"
        "end\n"
        "\n"
        "return fib(8)\n",
        {
            lvnumber(21),
        });
}