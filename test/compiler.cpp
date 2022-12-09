#include <compiler.hpp>
#include <parser.hpp>
#include <resolve.hpp>
#include <map>
#include <cstring>
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
    char *message = (char *)malloc(mlen);
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
        free(mes);
        return *this;
    }
    GenTest &test_hookmax(size_t hookmax)
    {
        char *mes = compiler_test_message(this->message, "hook LIFO max size");
        test_case(mes, this->test->hookmax == hookmax);
        free(mes);
        return *this;
    }
    GenTest &test_ccount(size_t ccount)
    {
        char *mes = compiler_test_message(this->message, "constant count");
        test_case(mes, this->test->ccount == ccount);
        free(mes);
        return *this;
    }
    GenTest &test_opcodes(vector<Opcode> opcodes)
    {
        vector<lbyte> text;
        for (size_t i = 0; i < opcodes.size(); i++)
        {
            Opcode op = opcodes[i];
            for (size_t j = 0; j < op.count; j++)
            {
                text.push_back(op.bytes[j]);
            }
        }
        char *mes = compiler_test_message(this->message, "text");
        test_case(mes, this->test->text == text);
        free(mes);
        return *this;
    }
    GenTest &test_upvalues(vector<Upvalue> upvalues)
    {
        char *mes = compiler_test_message(this->message, "upvalue table");
        test_case(mes, this->test->upvalues == upvalues);
        free(mes);
        return *this;
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
            Opcode(IConst, 0),
            Opcode(IPop, 1),
            Opcode(IRet, 0),
        });
}