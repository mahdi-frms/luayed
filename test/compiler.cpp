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