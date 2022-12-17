#include <compiler.hpp>
#include <parser.hpp>
#include <resolve.hpp>
#include <map>
#include <cstring>
#include <iostream>
#include <tap/tap.h>

struct FuncTest
{
    FuncTest *prev;
    vector<lbyte> text;
    vector<Upvalue> upvalues;
    size_t ccount;
    size_t hookmax;
    size_t parcount;
    fidx_t fidx;
};

typedef void (*test_case_function_t)(const char *, bool);

void test_case(const char *mes, bool rsl)
{
    ok(rsl, mes);
}

char *compiler_test_message(const char *item, const char *property)
{
    const char *title = "compiler : ";
    size_t ilen = strlen(item);
    size_t plen = strlen(property);
    size_t tlen = strlen(title);
    size_t mlen = ilen + plen + tlen + 4 /* ()\0*/;
    char *message = new char[mlen];
    strcpy(message, title);
    strcpy(message + tlen, item);
    message[tlen + ilen] = ' ';
    message[tlen + ilen + 1] = '(';
    strcpy(message + tlen + ilen + 2, property);
    message[tlen + ilen + plen + 2] = ')';
    message[tlen + ilen + plen + 3] = '\0';
    return message;
}

class GenTest final : IGenerator
{
private:
    std::map<fidx_t, FuncTest *> funcs;
    FuncTest *current;
    const char *message;
    FuncTest *test;

public:
    GenTest(const char *message) : current(nullptr), message(message), test(nullptr)
    {
    }
    void emit(Opcode opcode)
    {
        for (size_t i = 0; i < opcode.count; i++)
            this->current->text.push_back(opcode.bytes[i]);
    }
    size_t len()
    {
        return this->current->text.size();
    }
    void seti(size_t idx, lbyte b)
    {
        this->current->text[idx] = b;
    }
    size_t const_number(lnumber num)
    {
        return this->current->ccount++;
    }
    size_t const_string(const char *str)
    {
        return this->current->ccount++;
    }
    void pushf(fidx_t fidx)
    {
        FuncTest *fnt = new FuncTest();
        fnt->ccount = 0;
        fnt->prev = nullptr;
        fnt->hookmax = 0;
        fnt->parcount = 0;
        fnt->prev = this->current;
        fnt->fidx = fidx;
        this->current = fnt;
    }
    void popf()
    {
        this->funcs[this->current->fidx] = this->current;
        this->current = this->current->prev;
    }
    size_t upval(fidx_t fidx, size_t offset)
    {
        size_t idx = this->current->upvalues.size();
        this->current->upvalues.push_back(Upvalue{.fidx = fidx, .offset = offset});
        return idx;
    }
    void meta_parcount(size_t parcount)
    {
        this->current->parcount = parcount;
    }
    void meta_hookmax(size_t hookmax)
    {
        this->current->hookmax = hookmax;
    }

    GenTest &test_fn(size_t fidx)
    {
        this->test = this->funcs[fidx];
        return *this;
    }
    GenTest &test_parcount(size_t parcount)
    {
        char *mes = compiler_test_message(this->message, "parameter count");
        test_case(mes, this->test->parcount == parcount);
        delete[] mes;
        return *this;
    }
    GenTest &test_hookmax(size_t hookmax)
    {
        char *mes = compiler_test_message(this->message, "hook LIFO max size");
        test_case(mes, this->test->hookmax == hookmax);
        delete[] mes;
        return *this;
    }
    GenTest &test_ccount(size_t ccount)
    {
        char *mes = compiler_test_message(this->message, "constant count");
        test_case(mes, this->test->ccount == ccount);
        delete[] mes;
        return *this;
    }
    vector<lbyte> assemble(vector<Opcode> &opcodes)
    {
        vector<lbyte> bin;
        for (size_t i = 0; i < opcodes.size(); i++)
        {
            for (lbyte j = 0; j < opcodes[i].count; j++)
                bin.push_back(opcodes[i].bytes[j]);
        }
        return bin;
    }
    GenTest &test_opcodes(vector<Opcode> opcodes)
    {
        vector<lbyte> bin = this->assemble(opcodes);
        char *mes = compiler_test_message(this->message, "text");
        vector<lbyte> &gen = this->test->text;
        bool rsl = bin == this->test->text;
        test_case(mes, rsl);
        if (!rsl)
        {
            std::cerr << "generated and expected binaries do not match!\n"
                      << "expected binary:\n"
                      << binary_stringify(&bin[0], bin.size())
                      << "generated binary:\n"
                      << binary_stringify(&gen[0], gen.size());
        }
        delete[] mes;
        return *this;
    }
    GenTest &test_upvalues(vector<Upvalue> upvalues)
    {
        char *mes = compiler_test_message(this->message, "upvalue table");
        test_case(mes, this->test->upvalues == upvalues);
        delete[] mes;
        return *this;
    }
    ~GenTest()
    {
        for (auto it = this->funcs.begin(); it != this->funcs.end(); it++)
            delete it->second;
    }
};

GenTest compiler_test_case(const char *message, const char *text)
{
    GenTest gentest(message);
    Lexer lexer(text);
    Parser parser((ILexer *)&lexer);
    Ast ast = parser.parse();
    if (ast.root() == nullptr)
    {
        std::cerr << "parsing code failed [ " << message << "]\n"
                  << parser.get_error() << "\n";
        exit(1);
    }
    SemanticAnalyzer analyzer(ast);
    analyzer.analyze();
    Compiler compiler((IGenerator *)&gentest);
    compiler.compile(ast);
    ast.destroy();
    return gentest;
}

void compiler_tests()
{
    compiler_test_case(
        "simple local assignment",

        "local a = 3")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "two declaration",

        "local a,b = 3,4")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            iconst(1),
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "declaration without expression list",

        "local a,b")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            inil,
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "declaration with less expressions",

        "local a,b = 6")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            inil,
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "declaration with more expressions",

        "local a,b = 6,'', true ")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            iconst(1),
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "global assignment",

        "a = 3")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            iconst(1),
            igset,
            iret(0),
        });

    compiler_test_case(
        "double assignment",

        "a,b = 3,true")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(3)
        .test_upvalues({})
        .test_opcodes({
            // lvalues
            iconst(0),
            inil,
            iconst(1),
            inil,
            // rvalues
            iconst(2),
            itrue,
            // write to buffers
            iblstore(3),
            iblstore(4),
            // global set
            igset,
            igset,
            // end
            iret(0),
        });

    compiler_test_case(
        "double assignment with less expressions",

        "a,b = 3")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(3)
        .test_upvalues({})
        .test_opcodes({
            // lvalues
            iconst(0),
            inil,
            iconst(1),
            inil,
            // rvalues
            iconst(2),
            inil,
            // write to buffers
            iblstore(3),
            iblstore(4),
            // global set
            igset,
            igset,
            // end
            iret(0),
        });

    compiler_test_case(
        "double assignment with more expressions",

        "a,b = 3, 'str' , false")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(4)
        .test_upvalues({})
        .test_opcodes({
            // lvalues
            iconst(0),
            inil,
            iconst(1),
            inil,
            // rvalues
            iconst(2),
            iconst(3),
            // write to buffers
            iblstore(3),
            iblstore(4),
            // global set
            igset,
            igset,
            // end
            iret(0),
        });

    compiler_test_case(
        "local assignment",

        "local a\n"
        "a = 4\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            inil,
            iconst(0),
            ilstore(0),
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "multiple local assignments",

        "local a\n"
        "local b\n"
        "a = 4\n"
        "b = true\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            inil,
            inil,
            iconst(0),
            ilstore(0),
            itrue,
            ilstore(1),
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "varlist local assignment",

        "local a,b,c\n"
        "a,b,c = 1,3,8\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(3)
        .test_upvalues({})
        .test_opcodes({
            inil,
            inil,
            inil,
            // lvalues
            inil,
            inil,
            inil,
            // rvalues
            iconst(0),
            iconst(1),
            iconst(2),
            // write to buffers
            iblstore(4),
            iblstore(4),
            iblstore(4),
            // local set
            ilstore(2),
            ilstore(1),
            ilstore(0),
            // end
            ipop(3),
            iret(0),
        });

    compiler_test_case(
        "get globals",

        "local v = a")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            igget,
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "get local",

        "local foo\n"
        "local bar = foo\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            ilocal(0),
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "function call",

        "a()\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            igget,
            icall(0, 1),
            // end
            iret(0),
        });

    compiler_test_case(
        "function call with return value",

        "local a = b()\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            igget,
            icall(0, 2),
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "function call with multiple return values",

        "local a,b,c = d()\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            igget,
            icall(0, 4),
            ipop(3),
            iret(0),
        });

    compiler_test_case(
        "function call returning one value for varlist",

        "local a,b,c = d() , 3\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            igget,
            icall(0, 2),
            iconst(1),
            inil,
            ipop(3),
            iret(0),
        });

    compiler_test_case(
        "binary operators",

        "local a,b,c\n"

        "a=b+c\n"
        "a=b-c\n"
        "a=b*c\n"
        "a=b/c\n"
        "a=b//c\n"
        "a=b%c\n"
        "a=b^c\n"

        "a=b&c\n"
        "a=b|c\n"
        "a=b~c\n"
        "a=b>>c\n"
        "a=b<<c\n"

        "a=b..c\n"

        "a=b>c\n"
        "a=b<c\n"
        "a=b>=c\n"
        "a=b<=c\n"
        "a=b==c\n"
        "a=b~=c\n"

        "")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            // decl
            inil,
            inil,
            inil,
            // add
            ilocal(1),
            ilocal(2),
            iadd,
            ilstore(0),
            // sub
            ilocal(1),
            ilocal(2),
            isub,
            ilstore(0),
            // mult
            ilocal(1),
            ilocal(2),
            imult,
            ilstore(0),
            // float div
            ilocal(1),
            ilocal(2),
            ifltdiv,
            ilstore(0),
            // floor div
            ilocal(1),
            ilocal(2),
            iflrdiv,
            ilstore(0),
            // mod
            ilocal(1),
            ilocal(2),
            imod,
            ilstore(0),
            // pow
            ilocal(1),
            ilocal(2),
            ipow,
            ilstore(0),
            // binary and
            ilocal(1),
            ilocal(2),
            iband,
            ilstore(0),
            // binary or
            ilocal(1),
            ilocal(2),
            ibor,
            ilstore(0),
            // binary xor
            ilocal(1),
            ilocal(2),
            ibxor,
            ilstore(0),
            // binary right shift
            ilocal(1),
            ilocal(2),
            ishr,
            ilstore(0),
            // binary left shift
            ilocal(1),
            ilocal(2),
            ishl,
            ilstore(0),
            // concat
            ilocal(1),
            ilocal(2),
            iconcat,
            ilstore(0),
            // greater
            ilocal(1),
            ilocal(2),
            igt,
            ilstore(0),
            // less
            ilocal(1),
            ilocal(2),
            ilt,
            ilstore(0),
            // greater equal
            ilocal(1),
            ilocal(2),
            ige,
            ilstore(0),
            // less equal
            ilocal(1),
            ilocal(2),
            ile,
            ilstore(0),
            // equal
            ilocal(1),
            ilocal(2),
            ieq,
            ilstore(0),
            // not equal
            ilocal(1),
            ilocal(2),
            ine,
            ilstore(0),
            // end
            ipop(3),
            iret(0),
        });

    compiler_test_case(
        "expression with multiple binary operators",

        "local a = 1 + 2 + 3")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(3)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            iconst(1),
            iadd,
            iconst(2),
            iadd,
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "unary operators",

        "local a = not ~#-(4)")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            inegate,
            ilength,
            ibnot,
            inot,
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "nested block",

        "local a, b\n"
        "do\n"
        "    local c, d = 3, true\n"
        "end\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            inil,
            inil,
            iconst(0),
            itrue,
            ipop(2),
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "double nested block",

        "local a, b\n"
        "do\n"
        "    local c = \"hello world!\"\n"
        "    do\n"
        "       local d, e, f = 3, true\n"
        "    end\n"
        "end\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            inil,
            inil,
            iconst(0),
            iconst(1),
            itrue,
            inil,
            ipop(3),
            ipop(1),
            ipop(2),
            iret(0),
        });
}