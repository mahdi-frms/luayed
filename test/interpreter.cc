#include <iostream>
#include <lyddef.h>
#include <luabin.h>
#include <interpreter.h>
#include <generator.h>
#include <generator.h>
#include "test.h"
#include "mockruntime.h"
#include "lstrep.h"

class InterpreterTestCase
{
private:
    MockRuntime rt;
    int fault_code = 0;
    size_t retarg = 0;
    const char *message;

    void test(bool rsl, const char *suffix)
    {
        string mes = "interpreter : ";
        mes.append(message);
        mes.append(" ");
        mes.append(suffix);
        test_assert(rsl, mes.c_str());
    }

public:
    InterpreterTestCase(const char *message) : message(message)
    {
    }
    InterpreterTestCase &set_stack(vector<LuaValue> stack)
    {
        this->rt.set_stack(stack);
        return *this;
    }
    InterpreterTestCase &set_constants(vector<LuaValue> constants)
    {
        this->rt.set_constants(constants);
        return *this;
    }
    InterpreterTestCase &set_args(vector<LuaValue> args)
    {
        this->rt.set_args(args);
        return *this;
    }
    InterpreterTestCase &set_text(vector<Opcode> text)
    {
        if (!text.size() || (text.back().bytes[0] & 0b11111110) != Instruction::IRet)
        {
            std::cerr << "instructions at text '" << this->message << "' do not end with ret!\n";
            exit(1);
        }
        this->rt.set_text(text);
        return *this;
    }
    InterpreterTestCase &add_upvalue(LuaValue value)
    {
        this->rt.add_upvalue(value);
        return *this;
    }
    InterpreterTestCase &add_detached_upvalue(LuaValue value)
    {
        this->rt.add_detached_upvalue(value);
        return *this;
    }
    InterpreterTestCase &test_top()
    {
        const char *suffix = "(stack top)";
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
    InterpreterTestCase &test_stack(vector<LuaValue> expected_stack)
    {
        bool rsl = this->rt.get_stack() == expected_stack;
        this->test(rsl, "(stack elements)");
        if (!rsl)
        {
            std::cout << "stack:\n"
                      << this->rt.get_stack()
                      << "expected:\n"
                      << expected_stack;
        }
        return *this;
    }
    InterpreterTestCase &execute()
    {
        const char *suffix = "(execution)";
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
    InterpreterTestCase &execute(vector<Opcode> opcodes)
    {
        const char *suffix = "(execution)";
        Interpreter intp;
        intp.config_error_metadata(false);
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
    InterpreterTestCase &test_ret(size_t retc)
    {
        this->test(this->retarg == retc, "(return count)");
        return *this;
    }
    InterpreterTestCase &test_call_luafn(fidx_t fidx)
    {
        this->test(this->rt.icp_luafn.check(fidx, 0), "(call [luafn])");
        return *this;
    }
    InterpreterTestCase &test_call_fncall(size_t argc, size_t retc)
    {
        this->test(this->rt.icp_fncall.check(argc, retc), "(call [fncall])");
        return *this;
    }
    InterpreterTestCase &test_call_hookpush()
    {
        this->test(this->rt.icp_hookpush.check(0, 0), "(call [hookpush])");
        return *this;
    }
    InterpreterTestCase &test_call_hookpop()
    {
        this->test(this->rt.icp_hookpop.check(0, 0), "(call [hookpop])");
        return *this;
    }
    InterpreterTestCase &test_upvalue(size_t idx, LuaValue value)
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
        this->test(rsl, "(upvalue)");
        return *this;
    }
    InterpreterTestCase &test_error(Lerror err)
    {
        LuaValue expected = lvstring(to_string(err, true).c_str());
        LuaValue thrown = this->rt.get_error();
        bool rsl = this->rt.error_raised() && thrown == expected;
        this->test(rsl, "(error)");
        if (!rsl)
        {
            if (this->rt.error_raised())
            {
                std::cerr << "No error was raised\n";
            }
            else
            {
                std::cerr << "thrown:\n"
                          << thrown << "\nexpected:\n"
                          << expected << "\n";
            }
        }
        return *this;
    }
};

void interpreter_tests()
{
    InterpreterTestCase("push true")
        .execute({
            itrue,
        })
        .test_stack({
            lvbool(true),
        });

    InterpreterTestCase("push false")
        .execute({
            ifalse,
        })
        .test_stack({
            lvbool(false),
        });

    InterpreterTestCase("push nil")
        .execute({
            inil,
        })
        .test_stack({
            lvnil(),
        });

    InterpreterTestCase("pop value")
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

    InterpreterTestCase("pop multiple values")
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

    InterpreterTestCase("push constants")
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

    InterpreterTestCase("push arg")
        .set_args({
            lvbool(true),
            lvnumber(10),
            lvbool(false),
        })
        .set_stack({
            lvnil(),
        })
        .execute({
            ivargs(3),
        })
        .test_stack({
            lvnil(),
            lvbool(true),
            lvnumber(10),
        });

    InterpreterTestCase("push function")
        .execute({
            ifconst(1),
        })
        .test_call_luafn(1);

    InterpreterTestCase("push new hook")
        .execute({
            iupush,
        })
        .test_call_hookpush();

    InterpreterTestCase("pop hook")
        .execute({
            iupop,
        })
        .test_call_hookpop();

    InterpreterTestCase("ret")
        .execute({
            iret(5),
        })
        .test_ret(5);

    InterpreterTestCase("call")
        .set_text({
            icall(4, 2),
            iret(0),
        })
        .execute()
        .test_call_fncall(4, 2);

    InterpreterTestCase("local")
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

    InterpreterTestCase("store")
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

    InterpreterTestCase("back local")
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

    InterpreterTestCase("back store")
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

    InterpreterTestCase("jump")
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

    InterpreterTestCase("conditional jump")
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

    InterpreterTestCase("upvalue")
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

    InterpreterTestCase("upvalue store")
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

    InterpreterTestCase("eq/non-equal/number")
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

    InterpreterTestCase("eq/equal/number")
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

    InterpreterTestCase("eq/non-equal/bool")
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

    InterpreterTestCase("eq/equal/bool")
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

    InterpreterTestCase("eq/equal/nil")
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

    InterpreterTestCase("eq/equal/empty strings")
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

    InterpreterTestCase("eq/equal/strings")
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

    InterpreterTestCase("eq/non-equal/strings")
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

    InterpreterTestCase("eq/non-equal(case)/strings")
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

    InterpreterTestCase("eq/non-equal/tables")
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

    InterpreterTestCase("eq/equal/tables")
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

    InterpreterTestCase("eq/non-equal/different")
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

    InterpreterTestCase("ne/non-equal/different")
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

    InterpreterTestCase("gt/numbers")
        .set_stack({
            lvnumber(6),
            lvnumber(5),
            lvnumber(4),
        })
        .execute({
            ilocal(0),
            ilocal(1),
            igt,
            ilocal(1),
            ilocal(1),
            igt,
            ilocal(2),
            ilocal(1),
            igt,
        })
        .test_stack({
            lvnumber(6),
            lvnumber(5),
            lvnumber(4),
            lvbool(true),
            lvbool(false),
            lvbool(false),
        });

    InterpreterTestCase("ge/numbers")
        .set_stack({
            lvnumber(6),
            lvnumber(5),
            lvnumber(4),
        })
        .execute({
            ilocal(0),
            ilocal(1),
            ige,
            ilocal(1),
            ilocal(1),
            ige,
            ilocal(2),
            ilocal(1),
            ige,
        })
        .test_stack({
            lvnumber(6),
            lvnumber(5),
            lvnumber(4),
            lvbool(true),
            lvbool(true),
            lvbool(false),
        });

    InterpreterTestCase("lt/numbers")
        .set_stack({
            lvnumber(6),
            lvnumber(5),
            lvnumber(4),
        })
        .execute({
            ilocal(0),
            ilocal(1),
            ilt,
            ilocal(1),
            ilocal(1),
            ilt,
            ilocal(2),
            ilocal(1),
            ilt,
        })
        .test_stack({
            lvnumber(6),
            lvnumber(5),
            lvnumber(4),
            lvbool(false),
            lvbool(false),
            lvbool(true),
        });

    InterpreterTestCase("le/numbers")
        .set_stack({
            lvnumber(6),
            lvnumber(5),
            lvnumber(4),
        })
        .execute({
            ilocal(0),
            ilocal(1),
            ile,
            ilocal(1),
            ilocal(1),
            ile,
            ilocal(2),
            ilocal(1),
            ile,
        })
        .test_stack({
            lvnumber(6),
            lvnumber(5),
            lvnumber(4),
            lvbool(false),
            lvbool(true),
            lvbool(true),
        });

    InterpreterTestCase("gt/string")
        .set_stack({
            lvstring("luo"),
            lvstring("lue"),
            lvstring("lua"),
        })
        .execute({
            ilocal(0),
            ilocal(1),
            igt,
            ilocal(1),
            ilocal(1),
            igt,
            ilocal(2),
            ilocal(1),
            igt,
        })
        .test_stack({
            lvstring("luo"),
            lvstring("lue"),
            lvstring("lua"),
            lvbool(true),
            lvbool(false),
            lvbool(false),
        });

    InterpreterTestCase("ge/string")
        .set_stack({
            lvstring("luo"),
            lvstring("lue"),
            lvstring("lua"),
        })
        .execute({
            ilocal(0),
            ilocal(1),
            ige,
            ilocal(1),
            ilocal(1),
            ige,
            ilocal(2),
            ilocal(1),
            ige,
        })
        .test_stack({
            lvstring("luo"),
            lvstring("lue"),
            lvstring("lua"),
            lvbool(true),
            lvbool(true),
            lvbool(false),
        });

    InterpreterTestCase("lt/string")
        .set_stack({
            lvstring("luo"),
            lvstring("lue"),
            lvstring("lua"),
        })
        .execute({
            ilocal(0),
            ilocal(1),
            ilt,
            ilocal(1),
            ilocal(1),
            ilt,
            ilocal(2),
            ilocal(1),
            ilt,
        })
        .test_stack({
            lvstring("luo"),
            lvstring("lue"),
            lvstring("lua"),
            lvbool(false),
            lvbool(false),
            lvbool(true),
        });

    InterpreterTestCase("le/string")
        .set_stack({
            lvstring("luo"),
            lvstring("lue"),
            lvstring("lua"),
        })
        .execute({
            ilocal(0),
            ilocal(1),
            ile,
            ilocal(1),
            ilocal(1),
            ile,
            ilocal(2),
            ilocal(1),
            ile,
        })
        .test_stack({
            lvstring("luo"),
            lvstring("lue"),
            lvstring("lua"),
            lvbool(false),
            lvbool(true),
            lvbool(true),
        });

    InterpreterTestCase("concat")
        .set_stack({
            lvstring("foo"),
            lvstring("bar"),
        })
        .execute({
            iconcat,
        })
        .test_stack({
            lvstring("foobar"),
        });

    InterpreterTestCase("len")
        .set_stack({
            lvstring("foo"),
        })
        .execute({
            ilength,
        })
        .test_stack({
            lvnumber(3),
        });

    InterpreterTestCase("add")
        .set_stack({
            lvnumber(3),
            lvnumber(5),
        })
        .execute({
            iadd,
        })
        .test_stack({
            lvnumber(8),
        });

    InterpreterTestCase("subtract")
        .set_stack({
            lvnumber(3),
            lvnumber(5),
        })
        .execute({
            isub,
        })
        .test_stack({
            lvnumber(-2),
        });

    InterpreterTestCase("negate")
        .set_stack({
            lvnumber(7),
        })
        .execute({
            inegate,
        })
        .test_stack({
            lvnumber(-7),
        });

    InterpreterTestCase("multiply")
        .set_stack({
            lvnumber(-3),
            lvnumber(5),
        })
        .execute({
            imult,
        })
        .test_stack({
            lvnumber(-15),
        });

    InterpreterTestCase("floor division")
        .set_stack({
            lvnumber(12),
            lvnumber(5),
        })
        .execute({
            iflrdiv,
        })
        .test_stack({
            lvnumber(2),
        });

    InterpreterTestCase("float division")
        .set_stack({
            lvnumber(12),
            lvnumber(5),
        })
        .execute({
            ifltdiv,
        })
        .test_stack({
            lvnumber(2.4),
        });

    InterpreterTestCase("modulo")
        .set_stack({
            lvnumber(8),
            lvnumber(3),
        })
        .execute({
            imod,
        })
        .test_stack({
            lvnumber(2),
        });

    InterpreterTestCase("power")
        .set_stack({
            lvnumber(6),
            lvnumber(3),
        })
        .execute({
            ipow,
        })
        .test_stack({
            lvnumber(216),
        });

    InterpreterTestCase("to string")
        .set_stack({
            lvnumber(10),
            lvnumber(20),
        })
        .execute({
            iconcat,
        })
        .test_stack({
            lvstring("1020"),
        });

    InterpreterTestCase("parse string")
        .set_stack({
            lvstring("3"),
            lvstring("4"),
        })
        .execute({
            inegate,
            imult,
        })
        .test_stack({
            lvnumber(-12),
        });

    InterpreterTestCase("invalid arithmatic operand")
        .set_stack({
            lvnumber(3),
            lvnil(),
        })
        .execute({
            imult,
        })
        .test_error(error_invalid_operand(LuaType::LVNil));

    InterpreterTestCase("unconvertible string")
        .set_stack({
            lvstring("30"),
            lvstring("3o"),
        })
        .execute({
            imult,
        })
        .test_error(error_invalid_operand(LuaType::LVString));

    InterpreterTestCase("invalid comparison")
        .set_stack({
            lvnumber(3),
            lvstring("3"),
        })
        .execute({
            ile,
        })
        .test_error(error_invalid_comparison(LuaType::LVNumber, LuaType::LVString));

    InterpreterTestCase("invalid concat operand")
        .set_stack({
            lvstring("30"),
            lvbool(true),
        })
        .execute({
            iconcat,
        })
        .test_error(error_invalid_operand(LuaType::LVBool));

    InterpreterTestCase("invalid length operand")
        .set_stack({
            lvnumber(111),
        })
        .execute({
            ilength,
        })
        .test_error(error_invalid_operand(LuaType::LVNumber));

    InterpreterTestCase("basic table")
        .set_stack({
            lvnumber(1),         // key 1
            lvbool(true),        // key 2
            lvstring("3rd-key"), // key 3

            lvbool(false),         // value 1
            lvnumber(25),          // value 2
            lvstring("3rd-value"), // value 3
        })
        .execute({
            itnew,
            ilocal(0),
            ilocal(3),
            itset,
            ilocal(1),
            ilocal(4),
            itset,
            ilocal(2),
            ilocal(5),
            itset,

            iblocal(1),
            iblocal(1),

            ilocal(0),
            itget,
            ilstore(3),

            ilocal(1),
            itget,
            ilstore(4),

            ilocal(2),
            itget,
            ilstore(5),
        })
        // the same as before
        .test_stack({
            lvnumber(1),         // key 1
            lvbool(true),        // key 2
            lvstring("3rd-key"), // key 3

            lvbool(false),         // value 1
            lvnumber(25),          // value 2
            lvstring("3rd-value"), // value 3
        });

    InterpreterTestCase("table non-existing property")
        .set_stack({
            lvstring("1st-key"), // key
        })
        .execute({
            itnew,
            ilocal(0),
            itget,
        })
        .test_stack({
            lvstring("1st-key"), // key
            lvnil(),
        });

    InterpreterTestCase("table length")
        .set_stack({
            lvnumber(1), // key 1
            lvnumber(2), // key 2
            lvnumber(3), // key 4

            lvbool(false),         // value 1
            lvnumber(25),          // value 2
            lvstring("3rd-value"), // value 3
        })
        .execute({
            itnew,
            ilocal(0),
            ilocal(3),
            itset,
            ilocal(1),
            ilocal(4),
            itset,
            ilocal(2),
            ilocal(5),
            itset,
            ilength,

            ilstore(0),
            ipop(5),
        })
        .test_stack({
            lvnumber(3),
        });

    InterpreterTestCase("globals")
        .set_constants({
            lvstring("username"),
            lvstring("mahdi"),
        })
        .execute({
            iconst(0),
            iconst(1),
            igset,

            iconst(0),
            igget,
        })
        .test_stack({
            lvstring("mahdi"),
        });
}
