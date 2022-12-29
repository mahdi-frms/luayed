#ifndef INTERPRETER_h
#define INTERPRETER_h

#include "runtime.h"
#include "lerror.h"

class Interpreter;

typedef void (Interpreter::*opimpl)();

enum class InterpreterState
{
    Run,
    End,
    Error,
};

enum class Comparison
{
    GT,
    GE,
    LT,
    LE
};

class Interpreter : public IInterpreter
{
public:
    size_t run(IRuntime *rt);
    size_t run(IRuntime *rt, Opcode op);
    LError get_error();
    static void optable_init();
    Interpreter();

private:
    static opimpl optable[256];
    static bool is_initialized;

    size_t ip = 0;
    lbyte op;
    size_t arg1;
    size_t arg2;

    size_t retc = 0;
    LError error = error_ok();
    InterpreterState state = InterpreterState::Run;

    IRuntime *rt = nullptr;

    lbyte iread();
    size_t fetch(lbyte *bin);
    void exec();

    void push_bool(bool b);
    bool compare();
    bool compare(Comparison cmp);
    bool compare_number(LuaValue &a, LuaValue &b, Comparison cmp);
    bool compare_string(LuaValue &a, LuaValue &b, Comparison cmp);
    LuaValue hookread(Hook *hook);
    void hookwrite(Hook *hook, LuaValue value);

    void i_add();
    void i_sub();
    void i_mult();
    void i_flrdiv();
    void i_fltdiv();
    void i_mod();
    void i_pow();
    void i_concat();
    void i_bor();
    void i_band();
    void i_bxor();
    void i_shr();
    void i_shl();

    void i_len();
    void i_neg();
    void i_not();
    void i_bnot();

    void i_lt();
    void i_gt();
    void i_ge();
    void i_le();
    void i_eq();
    void i_ne();

    void i_tget();
    void i_tset();
    void i_tnew();
    void i_tlist();
    void i_gget();
    void i_gset();
    void i_nil();
    void i_true();
    void i_false();

    void i_ret();

    void i_call();
    void i_vargs();
    void i_jmp();
    void i_cjmp();

    void i_const();
    void i_fconst();

    void i_local();
    void i_lstore();
    void i_blocal();
    void i_blstore();
    void i_upvalue();
    void i_ustore();

    void i_upush();
    void i_upop();
    void i_pop();
};

#endif