#include <compiler.h>
#include <parser.h>
#include <resolve.h>
#include <map>
#include <cstring>
#include <iostream>
#include "test.h"
#include <lstrep.h>

struct FuncTest
{
    FuncTest *prev;
    vector<lbyte> text;
    vector<Upvalue> upvalues;
    vector<size_t> debug;
    size_t ccount;
    size_t hookmax;
    size_t parcount;
    fidx_t fidx;
};

typedef void (*test_case_function_t)(const char *, bool);

void test_case(const char *mes, bool rsl)
{
    test_assert(rsl, mes);
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

class GenTest final : public IGenerator
{
private:
    std::map<fidx_t, FuncTest *> funcs;
    FuncTest *current;
    const char *message;
    FuncTest *test;
    fidx_t fidx_counter = 1;

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
    void debug_info(size_t line)
    {
        while (this->current->debug.size() < this->current->text.size())
            this->current->debug.push_back(0);
        this->current->debug.back() = line;
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
    fidx_t pushf()
    {
        FuncTest *fnt = new FuncTest();
        fidx_t fidx = this->fidx_counter++;
        fnt->ccount = 0;
        fnt->prev = nullptr;
        fnt->hookmax = 0;
        fnt->parcount = 0;
        fnt->prev = this->current;
        fnt->fidx = fidx;
        this->current = fnt;
        return fidx;
    }
    void popf()
    {
        this->funcs[this->current->fidx] = this->current;
        this->current = this->current->prev;
    }
    size_t upval(fidx_t fidx, size_t offset, size_t hidx)
    {
        size_t idx = this->current->upvalues.size();
        this->current->upvalues.push_back(Upvalue(fidx, offset, hidx));
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
            std::cerr << "generated and expected binaries do not match!\nin: "
                      << this->message << "\n"
                      << "expected binary:\n"
                      << to_string(&bin[0], bin.size())
                      << "generated binary:\n"
                      << to_string(&gen[0], gen.size());
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
    Parser parser(&lexer);
    Ast ast = parser.parse();
    if (ast.root() == nullptr)
    {
        std::cerr << "parsing code failed [ " << message << "]\n"
                  << parser.get_error() << "\n";
        exit(1);
    }
    Resolver analyzer(ast);
    analyzer.analyze();
    Compiler compiler(&gentest);
    compiler.compile(ast);
    return gentest;
}

void compiler_tests()
{
    compiler_test_case(
        "simple declaration",

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
            iblstore(2),
            iblstore(3),
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
            iblstore(2),
            iblstore(3),
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
            iblstore(2),
            iblstore(3),
            // global set
            igset,
            igset,
            // end
            iret(0),
        });

    compiler_test_case(
        "globals and locals",

        "local a = 3\n"
        "v = 4\n"
        "a = v\n"
        "v = a\n"
        "")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(5)
        .test_upvalues({})
        .test_opcodes({
            // decl
            iconst(0),
            // assign 1
            iconst(1),
            iconst(2),
            igset,
            // assign 2
            iconst(3),
            igget,
            ilstore(0),
            // assign 3
            iconst(4),
            ilocal(0),
            igset,
            // end
            ipop(1),
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
            iblstore(3),
            iblstore(3),
            iblstore(3),
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
        "    a = 'text'\n"
        "    d = false\n"
        "end\n"
        "local e\n"
        "a = e\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            // decls
            inil,
            inil,
            iconst(0),
            itrue,
            // assignments
            iconst(1),
            ilstore(0),
            ifalse,
            ilstore(3),
            // end
            ipop(2),
            // decl
            inil,
            // assignment
            ilocal(2),
            ilstore(0),
            // chunk end
            ipop(3),
            iret(0),
        });

    compiler_test_case(
        "double nested block",

        "local a, b\n"
        "do\n"
        "    local c = \"hello world!\"\n"
        "    do\n"
        "       local d, e, f = 3, true\n"
        "       a = c + d\n"
        "    end\n"
        "end\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            // decl
            inil,
            inil,
            iconst(0),
            iconst(1),
            itrue,
            inil,
            // assignments
            ilocal(2),
            ilocal(3),
            iadd,
            ilstore(0),
            // end
            ipop(3),
            ipop(1),
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "if block",

        "local a ,b\n"
        "if a then\n"
        "    b()\n"
        "end\n")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            // decl
            inil,
            inil,
            ilocal(0),
            inot,
            icjmp(16),
            ilocal(1),
            icall(0, 1),
            ijmp(16),
            // end
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "if-else block",

        "local a, b, c\n"
        "if a then\n"
        "    b()\n"
        "else\n"
        "    c()\n"
        "end\n")

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
            // if
            ilocal(0),
            inot,
            icjmp(17),
            // then
            ilocal(1),
            icall(0, 1),
            ijmp(22),
            // else
            ilocal(2),
            icall(0, 1),
            // end
            ipop(3),
            iret(0),
        });

    compiler_test_case(
        "if-elseif-else block",

        "local a, b, c\n"
        "if true then\n"
        "    a()\n"
        "elseif false then\n"
        "    b()\n"
        "else\n"
        "    c()\n"
        "end\n")

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
            // if 3
            itrue,
            inot,
            icjmp(16),
            // then 8
            ilocal(0),
            icall(0, 1),
            ijmp(34),
            // elseif 16
            ifalse,
            inot,
            icjmp(29),
            // then 21
            ilocal(1),
            icall(0, 1),
            ijmp(34),
            // else 29
            ilocal(2),
            icall(0, 1),
            // end 34
            ipop(3),
            iret(0),
        });

    compiler_test_case(
        "logical or",

        "local a = false or 3")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            ifalse,
            iblocal(1),
            icjmp(10),
            ipop(1),
            iconst(0),
            // cjmp 10
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "logical and",

        "local a = true and 4")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            itrue,
            iblocal(1),
            inot,
            icjmp(11),
            ipop(1),
            iconst(0),
            // cjmp 11
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "while loop",

        "local a while (true) do a() end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            // while 1
            itrue,
            inot,
            icjmp(14),
            // do
            ilocal(0),
            icall(0, 1),
            ijmp(1),
            // end 14
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "repeat loop",

        "local a repeat a() until( false )")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            // repeat 1
            ilocal(0),
            icall(0, 1),
            // until
            ifalse,
            inot,
            icjmp(1),
            // end
            ipop(1),
            iret(0),
        });
    compiler_test_case(
        "numeric for",

        "local a\n"
        "for i = 0,10,2 do \n"
        "   local b\n"
        "   break\n"
        "   a(b,i)\n"
        "end\n"
        "local e\n"
        "a = e")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(3)
        .test_upvalues({})
        .test_opcodes({
            inil,
            // params 1
            iconst(0),
            iconst(1),
            iconst(2),
            // condition 7
            iblocal(3),
            iblocal(3),
            igt,
            icjmp(40),
            // block 15
            inil,
            ijmp(40),
            ilocal(0),
            ilocal(4),
            ilocal(1),
            icall(2, 1),
            ipop(1),
            // increment 30
            iblocal(3),
            iblocal(2),
            iadd,
            iblstore(3),
            ijmp(7),
            // loop end 40
            ipop(3),
            // end 42
            inil,
            ilocal(1),
            ilstore(0),
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "generic for",

        "local iter, state, prev, func\n"
        "for i,j,k in iter, state, prev do \n"
        "   local v\n"
        "   break\n"
        "   func(v,i,j)\n"
        "end\n"
        "local e\n"
        "state = e")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            // decls
            inil,
            inil,
            inil,
            inil,
            // explist
            ilocal(0),
            ilocal(1),
            ilocal(2),
            // varlist
            inil,
            inil,
            // swap
            iblocal(1),
            iblocal(4),
            iblstore(2),
            iblstore(3),
            iblocal(2),
            iblocal(3),
            iblstore(3),
            iblstore(2),
            iblocal(3),
            iblocal(2),
            iblstore(4),
            iblstore(1),
            // loop start 36
            iblocal(1),
            iblocal(3),
            ilocal(4),
            icall(2, 4),
            iblstore(5),
            iblstore(5),
            iblstore(5),
            // loop check
            ilocal(4),
            inil,
            ieq,
            icjmp(78),
            // block
            inil,
            ijmp(78),
            ilocal(3),
            ilocal(9),
            ilocal(4),
            ilocal(5),
            icall(3, 1),
            ipop(1),
            // end of block
            ijmp(36),
            // loop end 78
            ipop(5),
            // end of for
            inil,
            ilocal(4),
            ilstore(1),
            ipop(5),
            iret(0),
        });

    compiler_test_case(
        "function declaration",

        "local a = function() end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            ifconst(2),
            ipop(1),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            iret(0),
        });

    compiler_test_case(
        "function declaration with parameters",

        "local a = function(v,w) end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            ifconst(2),
            ipop(1),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(2)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            iret(0),
        });

    compiler_test_case(
        "function declaration with locals",

        "local a = function(v,w) local z v = z + w end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            ifconst(2),
            ipop(1),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(2)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            ilocal(2),
            ilocal(1),
            iadd,
            ilstore(0),
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "upvalue",

        "local u local a = function() local z = u end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(1)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            iupush,
            ifconst(2),
            iupop,
            ipop(2),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({
            Upvalue(1, 0, 0),
        })
        .test_opcodes({
            iupvalue(0),
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "upvalue in numeric for",

        "for i = 1,5,1 do local v = function() local w = i end end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(1)
        .test_ccount(3)
        .test_upvalues({})
        .test_opcodes({
            // params 0
            iconst(0),
            iupush,
            iconst(1),
            iconst(2),
            // condition 7
            iblocal(3),
            iblocal(3),
            igt,
            icjmp(29),
            // block 15
            ifconst(2),
            ipop(1),
            // increment 19
            iblocal(3), // counter
            iblocal(2), // step
            iadd,
            iblstore(3), // store new counter
            ijmp(7),
            // loop end 29
            iupop,
            ipop(3),
            // end
            iret(0),
        })

        .test_fn(2)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({
            Upvalue(1, 0, 0),
        })
        .test_opcodes({
            iupvalue(0),
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "upvalue store",

        "local u local a = function() u = 0 end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(1)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            iupush,
            ifconst(2),
            iupop,
            ipop(2),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({
            Upvalue(1, 0, 0),
        })
        .test_opcodes({
            iconst(0),
            iustore(0),
            iret(0),
        });

    compiler_test_case(
        "upvalue function params",

        "local function a(v1, vm, v2)\n"
        "    local function b()\n"
        "        v1 = v2\n"
        "    end\n"
        "end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            ifconst(2),
            ipop(1),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(3)
        .test_hookmax(2)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            iupush,
            iupush,
            ifconst(3),
            ipop(1),
            iupop,
            iupop,
            iret(0),
        })

        .test_fn(3)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({
            Upvalue(2, 0, 0),
            Upvalue(2, 2, 1),
        })
        .test_opcodes({
            iupvalue(1),
            iustore(0),
            iret(0),
        });

    compiler_test_case(
        "return statement",

        "return")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            iret(0),
            iret(0),
        });

    compiler_test_case(
        "return statement with args",

        "return 1 , 3")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            iconst(0),
            iconst(1),
            iret(2),
            iret(0),
        });

    compiler_test_case(
        "return statement with function call",

        "local f,g return 1 , f(), g()")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            // locals
            inil,
            inil,
            // 1st arg
            iconst(0),
            // 2nd args
            ilocal(0),
            icall(0, 2),
            // 3rd arg
            ilocal(1),
            icall(0, 0),
            // ret
            iret(2),
            // end
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "empty table constructor",

        "local a = {}")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            itnew,
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "table constructor with properties",

        "local a = { foo = 1 , bar = true }")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(3)
        .test_upvalues({})
        .test_opcodes({
            itnew,
            // foo
            iconst(0),
            iconst(1),
            itset,
            // bar
            iconst(2),
            itrue,
            itset,
            // end
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "table constructor props & indexes & expressions",

        "local i local a = { '' , foo = 1 , [i] = true , false}")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(5)
        .test_upvalues({})
        .test_opcodes({
            inil,
            itnew,
            // [1]
            iconst(0),
            iconst(1),
            itset,
            // foo
            iconst(2),
            iconst(3),
            itset,
            // bar
            ilocal(0),
            itrue,
            itset,
            // [2]
            iconst(4),
            ifalse,
            itset,
            // end
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "table constructor with extras",

        "local i local a = { 'str' , foo = 1 , true, i()}")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(5)
        .test_upvalues({})
        .test_opcodes({
            inil,
            itnew,
            // [1]
            iconst(0),
            iconst(1),
            itset,
            // foo
            iconst(2),
            iconst(3),
            itset,
            // [2]
            iconst(4),
            itrue,
            itset,
            // extra
            ilocal(0),
            icall(0, 0),
            // expressions and extra
            itlist(2),
            // end
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "table get",

        "local i local j = i.a + i[4]")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            inil,
            // 1st
            ilocal(0),
            iconst(0),
            itget,
            // 2st
            ilocal(0),
            iconst(1),
            itget,
            // operator
            iadd,
            // end
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "table set",

        "local i\n"
        "i[3] = true\n"
        "i()[7] = false")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            inil,
            // 1st
            ilocal(0),
            iconst(0),
            itrue,
            itset,
            ipop(1),
            // 2st
            ilocal(0),
            icall(0, 2),
            iconst(1),
            ifalse,
            itset,
            ipop(1),
            // end
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "varlist table set",

        "local i\n"
        "i[3] , i()[7] = true , false")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(2)
        .test_upvalues({})
        .test_opcodes({
            inil,
            // varlist
            // 1st
            ilocal(0),
            iconst(0),
            inil, // buffer
            // 2st
            ilocal(0),
            icall(0, 2),
            iconst(1),
            inil, // buffer
            // exprlist
            itrue,
            ifalse,
            // fill buffers
            iblstore(2),
            iblstore(4),
            // operators
            itset,
            itset,
            // pop
            ipop(2),
            // end
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "variable args",

        "local i,j,k = ...")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            ivargs(4),
            ipop(3),
            iret(0),
        });

    compiler_test_case(
        "variable args with unexpected count",

        "local i  i(...)")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            ilocal(0),
            ivargs(0),
            icall(0, 1),
            ipop(1),
            iret(0),
        });

    compiler_test_case(
        "function with variable params",

        "local function a(e,...) end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            ifconst(2),
            ipop(1),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(1)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            iret(0),
        });

    compiler_test_case(
        "method declaration",

        "local i = {} "
        "function i:f(a,b) "
        "   local c,d "
        "   d(a,b,c,self) "
        "end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            itnew,
            ilocal(0),
            iconst(0),
            ifconst(2),
            itset,
            ipop(1),
            ipop(1),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(3)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            inil,
            ilocal(4),
            ilocal(1),
            ilocal(2),
            ilocal(3),
            ilocal(0),
            icall(4, 1),
            ipop(2),
            iret(0),
        });
    compiler_test_case(
        "method declaration with 'self' as param",

        "local i = {} "
        "function i:f(self,a,b) "
        "   local c,d "
        "   d(a,b,c,self) "
        "end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            itnew,
            ilocal(0),
            iconst(0),
            ifconst(2),
            itset,
            ipop(1),
            ipop(1),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(4)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            inil,
            inil,
            ilocal(5),
            ilocal(2),
            ilocal(3),
            ilocal(4),
            ilocal(1),
            icall(4, 1),
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "method call",

        "local i,j "
        "i:a(j)")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(1)
        .test_upvalues({})
        .test_opcodes({
            inil,
            inil,
            // call
            inil,
            ilocal(0),
            iblocal(1),
            iconst(0),
            itget,
            iblstore(2),
            ilocal(1),
            icall(2, 1),
            // end
            ipop(2),
            iret(0),
        });

    compiler_test_case(
        "recursive function",

        "local function f() f() end")

        .test_fn(1)
        .test_parcount(0)
        .test_hookmax(1)
        .test_ccount(0)
        .test_upvalues({})
        .test_opcodes({
            iupush,
            ifconst(2),
            iupop,
            ipop(1),
            iret(0),
        })

        .test_fn(2)
        .test_parcount(0)
        .test_hookmax(0)
        .test_ccount(0)
        .test_upvalues({
            Upvalue(1, 0, 0),
        })
        .test_opcodes({
            iupvalue(0),
            icall(0, 1),
            iret(0),
        });
}