#ifndef INTERPRETOR_HPP
#define INTERPRETOR_HPP

#include "runtime.hpp"

class Interpretor;

typedef void (Interpretor::*opimpl)();

class Interpretor
{
public:
    size_t call(Lua *rt);

private:
    static opimpl optable[256];
    static void optable_init();

    size_t ip = 0;
    lbyte op;
    size_t arg1;
    size_t arg2;
    size_t retc = 0;
    bool end = false;
    Lua *rt = nullptr;

    void load(size_t argc, size_t retc);
    lbyte iread();
    void fetch();
    void exec();
    Frame *frame();
    Lfunction *bin();
    size_t sp();
    void setsp(size_t sp);
    LuaValue *stack(size_t idx);
    Hook *uptable(size_t idx);
    Hook *hooktable(size_t idx);

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
    void i_gget();
    void i_gset();
    void i_nil();
    void i_true();
    void i_false();

    void i_tlist();
    void i_ret();

    void i_fvargs();

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

    void i_push();
    void i_pop();
};

#endif