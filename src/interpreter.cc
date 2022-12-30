#include <cstring>
#include "virtuals.h"
#include "interpreter.h"

opimpl Interpreter::optable[256] = {};
bool Interpreter::is_initialized = false;

void Interpreter::optable_init()
{
    Interpreter::optable[IAdd] = &Interpreter::i_add;
    Interpreter::optable[ISub] = &Interpreter::i_sub;
    Interpreter::optable[IMult] = &Interpreter::i_mult;
    Interpreter::optable[IFlrDiv] = &Interpreter::i_flrdiv;
    Interpreter::optable[IFltDiv] = &Interpreter::i_fltdiv;
    Interpreter::optable[IMod] = &Interpreter::i_mod;
    Interpreter::optable[IPow] = &Interpreter::i_pow;
    Interpreter::optable[IConcat] = &Interpreter::i_concat;
    Interpreter::optable[IBOr] = &Interpreter::i_bor;
    Interpreter::optable[IBAnd] = &Interpreter::i_band;
    Interpreter::optable[IBXor] = &Interpreter::i_bxor;
    Interpreter::optable[ISHR] = &Interpreter::i_shr;
    Interpreter::optable[ISHL] = &Interpreter::i_shl;
    Interpreter::optable[ILength] = &Interpreter::i_len;
    Interpreter::optable[INegate] = &Interpreter::i_neg;
    Interpreter::optable[INot] = &Interpreter::i_not;
    Interpreter::optable[IBNot] = &Interpreter::i_bnot;
    Interpreter::optable[IEq] = &Interpreter::i_eq;
    Interpreter::optable[INe] = &Interpreter::i_ne;
    Interpreter::optable[IGe] = &Interpreter::i_ge;
    Interpreter::optable[IGt] = &Interpreter::i_gt;
    Interpreter::optable[ILe] = &Interpreter::i_le;
    Interpreter::optable[ILt] = &Interpreter::i_lt;
    Interpreter::optable[ITGet] = &Interpreter::i_tget;
    Interpreter::optable[ITSet] = &Interpreter::i_tset;
    Interpreter::optable[ITNew] = &Interpreter::i_tnew;
    Interpreter::optable[ITList] = &Interpreter::i_tlist;
    Interpreter::optable[IGGet] = &Interpreter::i_gget;
    Interpreter::optable[IGSet] = &Interpreter::i_gset;
    Interpreter::optable[INil] = &Interpreter::i_nil;
    Interpreter::optable[ITrue] = &Interpreter::i_true;
    Interpreter::optable[IFalse] = &Interpreter::i_false;
    Interpreter::optable[IRet] = &Interpreter::i_ret;
    Interpreter::optable[ICall] = &Interpreter::i_call;
    Interpreter::optable[IVargs] = &Interpreter::i_vargs;
    Interpreter::optable[IJmp] = &Interpreter::i_jmp;
    Interpreter::optable[ICjmp] = &Interpreter::i_cjmp;
    Interpreter::optable[IConst] = &Interpreter::i_const;
    Interpreter::optable[IConst] = &Interpreter::i_const;
    Interpreter::optable[IFConst] = &Interpreter::i_fconst;
    Interpreter::optable[ILocal] = &Interpreter::i_local;
    Interpreter::optable[ILStore] = &Interpreter::i_lstore;
    Interpreter::optable[IBLocal] = &Interpreter::i_blocal;
    Interpreter::optable[IBLStore] = &Interpreter::i_blstore;
    Interpreter::optable[IUpvalue] = &Interpreter::i_upvalue;
    Interpreter::optable[IUStore] = &Interpreter::i_ustore;
    Interpreter::optable[IUPush] = &Interpreter::i_upush;
    Interpreter::optable[IUPop] = &Interpreter::i_upop;
    Interpreter::optable[IPop] = &Interpreter::i_pop;
}

Interpreter::Interpreter()
{
    if (!Interpreter::is_initialized)
    {
        this->is_initialized = true;
        Interpreter::optable_init();
    }
}

size_t Interpreter::run(IRuntime *rt, Opcode op)
{
    this->rt = rt;
    this->fetch(op.bytes);
    this->exec();
    size_t retc = this->retc;
    this->retc = 0;
    this->state = InterpreterState::Run;
    return retc;
}
size_t Interpreter::run(IRuntime *rt)
{
    this->rt = rt;
    while (this->state == InterpreterState::Run)
    {
        this->ip += this->fetch(this->rt->text() + this->ip);
        this->exec();
    }
    size_t retc = this->retc;
    this->retc = 0;
    this->state = InterpreterState::Run;
    return retc;
}

size_t Interpreter::fetch(lbyte *bin)
{
    size_t ptr = 0;
    lbyte op = bin[ptr++];
    if (op & 0x80)
    {
        this->arg1 = bin[ptr++];
        if (op & 0x01)
        {
            op &= 0b11111110;
            this->arg1 += bin[ptr++] << 8;
        }

        if (op == Instruction::ICall)
        {
            this->arg2 = bin[ptr++];
            if (op & 0x02)
            {
                op &= 0b11111101;
                this->arg2 += bin[ptr++] << 8;
            }
        }
    }
    this->op = op;
    return ptr;
}

void Interpreter::exec()
{
    (this->*optable[this->op])();
}

void Interpreter::push_bool(bool b)
{
    this->rt->stack_push(this->rt->create_boolean(b));
}

void Interpreter::i_add()
{
    // todo: string conversion
    LuaValue a = this->rt->stack_pop();
    LuaValue b = this->rt->stack_pop();
    if (a.kind != LuaType::LVNumber || b.kind != LuaType::LVNumber)
    {
        this->generate_error(error_invalid_binary_operands(a.kind, b.kind));
        this->state = InterpreterState::Error;
        return;
    }
    LuaValue sum = this->rt->create_number(a.data.n + b.data.n);
    this->rt->stack_push(sum);
}
void Interpreter::i_sub()
{
    // todo: string conversion
    LuaValue a = this->rt->stack_pop();
    LuaValue b = this->rt->stack_pop();
    if (a.kind != LuaType::LVNumber || b.kind != LuaType::LVNumber)
    {
        this->generate_error(error_invalid_binary_operands(a.kind, b.kind));
        this->state = InterpreterState::Error;
        return;
    }
    LuaValue sum = this->rt->create_number(a.data.n - b.data.n);
    this->rt->stack_push(sum);
}
void Interpreter::i_mult()
{
}
void Interpreter::i_flrdiv()
{
}
void Interpreter::i_fltdiv()
{
}
void Interpreter::i_mod()
{
}
void Interpreter::i_pow()
{
}
void Interpreter::i_neg()
{
    // todo: string conversion
    LuaValue a = this->rt->stack_pop();
    if (a.kind != LuaType::LVNumber)
    {
        this->generate_error(error_invalid_unary_operand(a.kind));
        this->state = InterpreterState::Error;
        return;
    }
    LuaValue num = this->rt->create_number(-a.data.n);
    this->rt->stack_push(num);
}

void Interpreter::i_bor()
{
}
void Interpreter::i_band()
{
}
void Interpreter::i_bxor()
{
}
void Interpreter::i_bnot()
{
}
void Interpreter::i_shr()
{
}
void Interpreter::i_shl()
{
}

void Interpreter::i_not()
{
    LuaValue val = this->rt->stack_pop();
    bool rsl = !val.truth();
    this->push_bool(rsl);
}
void Interpreter::i_concat()
{
}
void Interpreter::i_len()
{
}

bool Interpreter::compare(Comparison cmp)
{
    LuaValue b = this->rt->stack_pop();
    LuaValue a = this->rt->stack_pop();
    bool rsl;
    if (a.kind != b.kind || (a.kind != LuaType::LVNumber && a.kind != LuaType::LVString))
    {
        this->generate_error(error_invalid_binary_operands(a.kind, b.kind));
        this->state = InterpreterState::Error;
        return false;
    }
    else if (a.kind == LuaType::LVNumber)
        rsl = this->compare_number(a, b, cmp);
    else
        rsl = this->compare_string(a, b, cmp);
    return rsl;
}
bool Interpreter::compare_number(LuaValue &a, LuaValue &b, Comparison cmp)
{
    if (cmp == Comparison::GE)
        return a.data.n >= b.data.n;
    if (cmp == Comparison::GT)
        return a.data.n > b.data.n;
    if (cmp == Comparison::LE)
        return a.data.n <= b.data.n;
    return a.data.n < b.data.n;
}
bool Interpreter::compare_string(LuaValue &a, LuaValue &b, Comparison cmp)
{
    if (cmp == Comparison::GE)
        return strcmp((char *)a.data.ptr, (char *)b.data.ptr) >= 0;
    if (cmp == Comparison::GT)
        return strcmp((char *)a.data.ptr, (char *)b.data.ptr) > 0;
    if (cmp == Comparison::LE)
        return strcmp((char *)a.data.ptr, (char *)b.data.ptr) <= 0;
    return strcmp((char *)a.data.ptr, (char *)b.data.ptr) < 0;
}
LuaValue Interpreter::hookread(Hook *hook)
{
    if (hook->is_detached)
    {
        return hook->val;
    }
    else
    {
        return *hook->original;
    }
}
void Interpreter::hookwrite(Hook *hook, LuaValue value)
{
    if (hook->is_detached)
    {
        hook->val = value;
    }
    else
    {
        *hook->original = value;
    }
}
void Interpreter::i_lt()
{
    this->push_bool(this->compare(Comparison::LT));
}
void Interpreter::i_gt()
{
    this->push_bool(this->compare(Comparison::GT));
}
void Interpreter::i_ge()
{
    this->push_bool(this->compare(Comparison::GE));
}
void Interpreter::i_le()
{
    this->push_bool(this->compare(Comparison::LE));
}
void Interpreter::i_eq()
{
    this->push_bool(this->compare());
}
void Interpreter::i_ne()
{
    this->push_bool(!this->compare());
}
bool Interpreter::compare()
{
    LuaValue a = this->rt->stack_pop();
    LuaValue b = this->rt->stack_pop();
    return a == b;
}
void Interpreter::i_tget()
{
}
void Interpreter::i_tset()
{
}
void Interpreter::i_tnew()
{
}
void Interpreter::i_tlist()
{
}
void Interpreter::i_gget()
{
}
void Interpreter::i_gset()
{
}
void Interpreter::i_nil()
{
    this->rt->stack_push(this->rt->create_nil());
}
void Interpreter::i_true()
{
    this->push_bool(true);
}
void Interpreter::i_false()
{
    this->push_bool(false);
}
void Interpreter::i_ret()
{
    this->retc = this->arg1;
    this->state = InterpreterState::End;
}
void Interpreter::i_call()
{
    this->rt->save_ip(this->ip);
    this->rt->fncall(this->arg1, this->arg2);
    this->ip = this->rt->load_ip();
}
void Interpreter::i_vargs()
{
    for (size_t i = 0; i < this->arg1; i++)
        this->rt->stack_push(this->rt->arg(i));
}
void Interpreter::i_jmp()
{
    this->ip = this->arg1;
}
void Interpreter::i_cjmp()
{
    LuaValue value = this->rt->stack_pop();
    if (value.truth())
        this->ip = this->arg1;
}
void Interpreter::i_const()
{
    LuaValue val = this->rt->rodata(this->arg1);
    this->rt->stack_push(val);
}
void Interpreter::i_fconst()
{
    LuaValue fn = this->rt->create_luafn(this->arg1);
    this->rt->stack_push(fn);
}
void Interpreter::i_local()
{
    LuaValue value = this->rt->stack_read(this->arg1);
    this->rt->stack_push(value);
}
void Interpreter::i_lstore()
{
    LuaValue value = this->rt->stack_pop();
    this->rt->stack_write(this->arg1, value);
}
void Interpreter::i_blocal()
{
    LuaValue value = this->rt->stack_back_read(this->arg1);
    this->rt->stack_push(value);
}
void Interpreter::i_blstore()
{
    LuaValue value = this->rt->stack_pop();
    this->rt->stack_back_write(this->arg1, value);
}
void Interpreter::i_upvalue()
{
    Hook *hook = this->rt->upvalue(this->arg1);
    LuaValue value = this->hookread(hook);
    this->rt->stack_push(value);
}
void Interpreter::i_ustore()
{
    Hook *hook = this->rt->upvalue(this->arg1);
    LuaValue value = this->rt->stack_pop();
    this->hookwrite(hook, value);
}
void Interpreter::i_upush()
{
    this->rt->hookpush();
}
void Interpreter::i_upop()
{
    this->rt->hookpop();
}
void Interpreter::i_pop()
{
    for (size_t i = 0; i < this->arg1; i++)
    {
        this->rt->stack_pop();
    }
}
void Interpreter::generate_error(LError error)
{
    LuaValue errval = this->rt->create_number(0xc0debed); // todo
    this->rt->set_error(errval);
}