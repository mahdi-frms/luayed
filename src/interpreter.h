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

enum class Calculation
{
    // Arithmetic
    CalcAdd,
    CalcSub,
    CalcMult,
    CalcFlrDiv,
    CalcFltDiv,
    CalcMod,
    CalcPow,
    // Binary
    CalcOr,
    CalcAnd,
    CalcXor,
    CalcNot,
    CalcSHR,
    CalcSHL,
};

class Interpreter : public IInterpreter
{
public:
    size_t run(IRuntime *rt);
    size_t run(IRuntime *rt, Instruction op);
    static void optable_init();
    Interpreter();
    void config_error_metadata(bool val);

private:
    static opimpl optable[256];
    static bool is_initialized;

    size_t ip = 0;
    size_t pip = 0;
    lbyte op;
    size_t arg1;
    size_t arg2;

    bool config_error_metadata_v = true;

    size_t retc = 0;
    InterpreterState state = InterpreterState::Run;

    IRuntime *rt = nullptr;

    lbyte iread();
    size_t fetch(lbyte *bin);
    void exec();

    bool check_whole(lnumber num);
    void push_bool(bool b);
    void generate_error(Lerror error);
    bool compare();
    void compare(Comparison cmp);
    bool compare_number(LuaValue &a, LuaValue &b, Comparison cmp);
    bool compare_string(LuaValue &a, LuaValue &b, Comparison cmp);
    LuaValue hookread(Hook *hook);
    void hookwrite(Hook *hook, LuaValue value);
    void arith(Calculation ar);
    void binary(Calculation bin);
    lnumber arith_calc(Calculation ar, lnumber a, lnumber b);
    int64_t bin_calc(Calculation bin, int64_t a, int64_t b);
    LuaValue parse_number(const char *str);
    LuaValue concat(LuaValue s1, LuaValue s2);
    LuaValue error_to_string(Lerror error);
    LuaValue lua_type_to_string(LuaType t);
    LuaValue error_add_meta(LuaValue e);

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