#include <iostream>
#include <luadef.h>
#include <luabin.h>
#include <interpreter.h>
#include <generator.h>
#include <generator.h>
#include <tap/tap.h>
#include "mockruntime.h"

#define IPTR(I) Opcode(0x00, I)

LuaValue vnumber(lnumber n)
{
    LuaValue v;
    v.kind = LuaType::LVNumber;
    v.data.n = n;
    return v;
}
LuaValue vstring(const char *s)
{
    LuaValue v;
    v.kind = LuaType::LVString;
    v.data.ptr = (void *)s;
    return v;
}
LuaValue vbool(bool b)
{
    LuaValue v;
    v.kind = LuaType::LVBool;
    v.data.b = b;
    return v;
}
LuaValue vnil()
{
    LuaValue v;
    v.kind = LuaType::LVNil;
    return v;
}

class InterpretorTestCase
{
private:
    MockRuntime rt;
    int fault_code = 0;
    size_t retarg = 0;
    const char *message;

    void test(bool rsl, const char *suffix)
    {
        string mes = message;
        mes.append(" : ");
        mes.append(suffix);
        ok(rsl, mes.c_str());
    }

public:
    InterpretorTestCase(const char *message) : message(message)
    {
    }
    InterpretorTestCase &set_stack(vector<LuaValue> stack)
    {
        this->rt.set_stack(stack);
        return *this;
    }
    InterpretorTestCase &set_constants(vector<LuaValue> constants)
    {
        this->rt.set_constants(constants);
        return *this;
    }
    InterpretorTestCase &set_args(vector<LuaValue> args)
    {
        this->rt.set_args(args);
        return *this;
    }
    InterpretorTestCase &set_text(vector<Opcode> text)
    {
        this->set_text(text);
        return *this;
    }
    InterpretorTestCase &test_top()
    {
        const char *suffix = "stack top";
        try
        {
            LuaValue top = this->rt.stack_pop();
            this->rt.stack_push(top);
            this->test(top.truth(), suffix);
        }
        catch (int fault_code)
        {
            this->test(false, suffix);
        }
        return *this;
    }
    void print_valvec(vector<LuaValue> &stack)
    {
        for (size_t i = 0; i < stack.size(); i++)
            std::cerr << stack[i] << "\n";
    }
    InterpretorTestCase &test_stack(vector<LuaValue> expected_stack)
    {
        bool rsl = this->rt.get_stack() == expected_stack;
        this->test(rsl, "stack elements");
        if (!rsl)
        {
            std::cout << "stack:\n";
            this->print_valvec(this->rt.get_stack());
            std::cout << "expected:\n";
            this->print_valvec(expected_stack);
        }
        return *this;
    }
    InterpretorTestCase &execute()
    {
        const char *suffix = "execution";
        Interpreter intp;
        try
        {
            this->retarg = intp.run((IRuntime *)&this->rt);
            this->test(true, suffix);
        }
        catch (int fault_code)
        {
            this->fault_code = fault_code;
            this->test(false, suffix);
            std::cerr << "execution failed with fault code : " << this->fault_code << "\n";
        }
        return *this;
    }
    InterpretorTestCase &execute(vector<Opcode> opcodes)
    {
        const char *suffix = "execution";
        Interpreter intp;
        try
        {
            for (size_t i = 0; i < opcodes.size(); i++)
                this->retarg = intp.run(&this->rt, opcodes[i]);
            this->test(true, suffix);
        }
        catch (int fault_code)
        {
            this->fault_code = fault_code;
            this->test(false, suffix);
            std::cerr << "execution failed with fault code : " << this->fault_code << "\n";
        }
        return *this;
    }
    InterpretorTestCase &test_ret(size_t retc)
    {
        this->test(this->retarg == retc, "return count");
        return *this;
    }
};

void interpreter_tests()
{
    InterpretorTestCase("push true")
        .execute({
            itrue,
        })
        .test_stack({
            lvbool(true),
        });

    InterpretorTestCase("push false")
        .execute({
            ifalse,
        })
        .test_stack({
            lvbool(false),
        });

    InterpretorTestCase("push nil")
        .execute({
            inil,
        })
        .test_stack({
            lvnil(),
        });

    InterpretorTestCase("pop value")
        .set_stack({
            lvnumber(3),
            lvbool(true),
            lvnumber(1),
        })
        .execute({
            ipop(1),
        })
        .test_stack({
            lvnumber(3),
            lvbool(true),
        });

    InterpretorTestCase("pop multiple values")
        .set_stack({
            lvnumber(3),
            lvbool(true),
            lvnumber(1),
            lvnil(),
        })
        .execute({
            ipop(3),
        })
        .test_stack({
            lvnumber(3),
        });

    InterpretorTestCase("push constants")
        .set_constants({
            lvnumber(2),
            lvnumber(10),
        })
        .set_stack({
            lvnumber(3),
        })
        .execute({
            iconst(1),
        })
        .test_stack({
            lvnumber(3),
            lvnumber(10),
        });
}
