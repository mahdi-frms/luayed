#include <iostream>
#include <luadef.h>
#include <luabin.h>
#include <interpreter.h>
#include <generator.h>
#include <generator.h>
#include <tap/tap.h>
#include "mockruntime.h"
#include "lstrep.h"

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
        if (!text.size() || (text.back().bytes[0] & 0b11111110) != Instruction::IRet)
        {
            std::cerr << "instructions at text '" << this->message << "' do not end with ret!\n";
            exit(1);
        }
        this->rt.set_text(text);
        return *this;
    }
    InterpretorTestCase &add_upvalue(LuaValue value)
    {
        this->rt.add_upvalue(value);
        return *this;
    }
    InterpretorTestCase &add_detached_upvalue(LuaValue value)
    {
        this->rt.add_detached_upvalue(value);
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
            this->retarg = intp.run(&this->rt);
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
    InterpretorTestCase &test_call_luafn(fidx_t fidx)
    {
        this->test(this->rt.icp_luafn.check(fidx, 0), "call [luafn]");
        return *this;
    }
    InterpretorTestCase &test_call_fncall(size_t argc, size_t retc)
    {
        this->test(this->rt.icp_fncall.check(argc, retc), "call [fncall]");
        return *this;
    }
    InterpretorTestCase &test_call_hookpush()
    {
        this->test(this->rt.icp_hookpush.check(0, 0), "call [hookpush]");
        return *this;
    }
    InterpretorTestCase &test_call_hookpop()
    {
        this->test(this->rt.icp_hookpop.check(0, 0), "call [hookpop]");
        return *this;
    }
    InterpretorTestCase &test_upvalue(size_t idx, LuaValue value)
    {
        Hook *hook = this->rt.upvalue(idx);
        if (!hook)
        {
            std::cerr << "invalid upvalue index (" << idx << ")\n";
            exit(1);
        }
        bool rsl;
        if (hook->is_detached)
        {
            rsl = hook->val == value;
        }
        else
        {
            rsl = *hook->original == value;
        }
        this->test(rsl, "upvalue");
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

    InterpretorTestCase("push arg")
        .set_args({
            lvbool(true),
            lvnumber(10),
            lvbool(false),
        })
        .set_stack({
            lvnil(),
        })
        .execute({
            ivargs(2),
        })
        .test_stack({
            lvnil(),
            lvbool(true),
            lvnumber(10),
        });

    InterpretorTestCase("push function")
        .execute({
            ifconst(1),
        })
        .test_call_luafn(1);

    InterpretorTestCase("push new hook")
        .execute({
            iupush,
        })
        .test_call_hookpush();

    InterpretorTestCase("pop hook")
        .execute({
            iupop,
        })
        .test_call_hookpop();

    InterpretorTestCase("ret")
        .execute({
            iret(5),
        })
        .test_ret(5);

    InterpretorTestCase("call")
        .set_text({
            icall(4, 2),
            iret(0),
        })
        .execute()
        .test_call_fncall(4, 2);

    InterpretorTestCase("local")
        .set_stack({
            lvnumber(5),
            lvbool(false),
            lvnumber(10),
        })
        .execute({
            ilocal(1),
            ilocal(3),
        })
        .test_stack({
            lvnumber(5),
            lvbool(false),
            lvnumber(10),
            lvbool(false),
            lvbool(false),
        });

    InterpretorTestCase("store")
        .set_stack({
            lvnumber(5),
            lvbool(false),
            lvnumber(10),
        })
        .execute({
            ilocal(1),
            ilstore(0),
        })
        .test_stack({
            lvbool(false),
            lvbool(false),
            lvnumber(10),
        });

    InterpretorTestCase("back local")
        .set_stack({
            lvnumber(5),
            lvbool(false),
            lvnumber(10),
        })
        .execute({
            iblocal(1),
        })
        .test_stack({
            lvnumber(5),
            lvbool(false),
            lvnumber(10),
            lvnumber(10),
        });

    InterpretorTestCase("back store")
        .set_stack({
            lvnumber(5),
            lvbool(false),
            lvnumber(10),
        })
        .execute({
            iblstore(2),
        })
        .test_stack({
            lvnumber(10),
            lvbool(false),
        });

    InterpretorTestCase("jump")
        .set_text({
            inil,
            ijmp(5),
            inil,
            inil,
            iret(0),
        })
        .execute()
        .test_stack({
            lvnil(),
            lvnil(),
        });

    InterpretorTestCase("conditional jump")
        .set_stack({
            lvnumber(1),
            lvnil(),
            lvnumber(0),
            lvbool(true),
        })
        .set_text({
            icjmp(0),
            iret(0),
        })
        .execute()
        .test_stack({
            lvnumber(1),
        });

    InterpretorTestCase("upvalue")
        .add_upvalue(lvnumber(7))
        .add_detached_upvalue(lvnumber(3))
        .add_detached_upvalue(lvnumber(8))
        .execute({
            iupvalue(0),
            iupvalue(2),
        })
        .test_stack({
            lvnumber(7),
            lvnumber(8),
        });

    InterpretorTestCase("upvalue store")
        .add_upvalue(lvnumber(7))
        .add_detached_upvalue(lvnumber(3))
        .add_detached_upvalue(lvnumber(8))
        .execute({
            itrue,
            iustore(2),
        })
        .test_stack({})
        .test_upvalue(0, lvnumber(7))
        .test_upvalue(1, lvnumber(3))
        .test_upvalue(2, lvbool(true));

    InterpretorTestCase("eq/non-equal/number")
        .set_stack({
            lvnumber(3),
            lvnumber(5),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(false),
        });

    InterpretorTestCase("eq/equal/number")
        .set_stack({
            lvnumber(5),
            lvnumber(5),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(true),
        });

    InterpretorTestCase("eq/non-equal/bool")
        .set_stack({
            lvbool(true),
            lvbool(false),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(false),
        });

    InterpretorTestCase("eq/equal/bool")
        .set_stack({
            lvbool(false),
            lvbool(false),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(true),
        });

    InterpretorTestCase("eq/equal/nil")
        .set_stack({
            lvnil(),
            lvnil(),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(true),
        });

    InterpretorTestCase("eq/equal/empty strings")
        .set_stack({
            lvstring(""),
            lvstring(""),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(true),
        });

    InterpretorTestCase("eq/equal/strings")
        .set_stack({
            lvstring("lua"),
            lvstring("lua"),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(true),
        });

    InterpretorTestCase("eq/non-equal/strings")
        .set_stack({
            lvstring("lua"),
            lvstring("luac"),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(false),
        });

    InterpretorTestCase("eq/non-equal(case)/strings")
        .set_stack({
            lvstring("lua"),
            lvstring("lUa"),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(false),
        });

    InterpretorTestCase("eq/non-equal/tables")
        .set_stack({
            lvtable(),
            lvtable(),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(false),
        });

    InterpretorTestCase("eq/equal/tables")
        .set_stack({
            lvtable(),
        })
        .execute({
            iblocal(1),
            ieq,
        })
        .test_stack({
            lvbool(true),
        });

    InterpretorTestCase("eq/non-equal/different")
        .set_stack({
            lvtable(),
            lvstring("lua"),
        })
        .execute({
            ieq,
        })
        .test_stack({
            lvbool(false),
        });

    InterpretorTestCase("ne/non-equal/different")
        .set_stack({
            lvtable(),
            lvstring("lua"),
        })
        .execute({
            ine,
        })
        .test_stack({
            lvbool(true),
        });
}
