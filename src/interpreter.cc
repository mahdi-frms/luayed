#include <cstring>
#include "virtuals.h"
#include "interpreter.h"
#include <cmath>
#include "lexer.h"

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
        Interpreter::is_initialized = true;
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
    this->ip = 0;
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

lnumber Interpreter::arith_calc(Arithmetic ar, lnumber a, lnumber b)
{
    if (ar == Arithmetic::ArAdd)
        return a + b;
    if (ar == Arithmetic::ArSub)
        return a - b;
    if (ar == Arithmetic::ArMult)
        return a * b;
    if (ar == Arithmetic::ArFltDiv)
        return a / b;
    if (ar == Arithmetic::ArFlrDiv)
        return floor(a / b);
    if (ar == Arithmetic::ArMod)
        return fmod(a, b);
    if (ar == Arithmetic::ArPow)
        return pow(a, b);
    return 0;
}

LuaValue Interpreter::parse_number(const char *str)
{
    Lexer lx(str);
    if (lx.next().kind != TokenKind::Number)
        return this->rt->create_nil();
    if (lx.next().kind != TokenKind::Eof)
        return this->rt->create_nil();
    return this->rt->create_number(atof(str));
}

void Interpreter::arith(Arithmetic ar)
{
    LuaValue b = this->rt->stack_pop();
    LuaValue a = this->rt->stack_pop();
    LuaType at = a.kind;
    LuaType bt = b.kind;

    if (at == LuaType::LVString)
        a = this->parse_number(a.as<const char *>());
    if (bt == LuaType::LVString)
        b = this->parse_number(b.as<const char *>());

    if (a.kind != LuaType::LVNumber)
    {
        return this->generate_error(error_invalid_operand(at));
    }
    if (b.kind != LuaType::LVNumber)
    {
        return this->generate_error(error_invalid_operand(bt));
    }

    lnumber rsl = this->arith_calc(ar, a.data.n, b.data.n);
    LuaValue rslval = this->rt->create_number(rsl);
    this->rt->stack_push(rslval);
}

void Interpreter::i_add()
{
    this->arith(Arithmetic::ArAdd);
}
void Interpreter::i_sub()
{
    this->arith(Arithmetic::ArSub);
}
void Interpreter::i_mult()
{
    this->arith(Arithmetic::ArMult);
}
void Interpreter::i_flrdiv()
{
    this->arith(Arithmetic::ArFlrDiv);
}
void Interpreter::i_fltdiv()
{
    this->arith(Arithmetic::ArFltDiv);
}
void Interpreter::i_mod()
{
    this->arith(Arithmetic::ArMod);
}
void Interpreter::i_pow()
{
    this->arith(Arithmetic::ArPow);
}
void Interpreter::i_neg()
{
    LuaValue a = this->rt->stack_pop();
    LuaType at = a.kind;
    if (a.kind == LuaType::LVString)
    {
        a = this->parse_number(a.as<const char *>());
    }
    if (a.kind != LuaType::LVNumber)
    {
        return this->generate_error(error_invalid_operand(at));
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
    LuaValue b = this->rt->stack_pop();
    LuaValue a = this->rt->stack_pop();

    if (a.kind == LuaType::LVNumber)
        a = this->rt->create_string(a.data.n);
    if (a.kind != LuaType::LVString)
    {
        return this->generate_error(error_invalid_operand(a.kind));
    }

    if (b.kind == LuaType::LVNumber)
        b = this->rt->create_string(b.data.n);
    if (b.kind != LuaType::LVString)
    {
        return this->generate_error(error_invalid_operand(b.kind));
    }

    LuaValue c = this->concat(a, b);
    this->rt->stack_push(c);
}
void Interpreter::i_len()
{
    LuaValue s = this->rt->stack_pop();
    if (s.kind == LuaType::LVString)
        this->rt->stack_push(this->rt->create_number(strlen(s.as<const char *>())));
    else if (s.kind == LuaType::LVTable)
    {
        LuaValue l = this->rt->create_number(1);
        while (this->rt->table_get(s, l) != this->rt->create_nil())
            l.data.n++;
        l.data.n--;
        this->rt->stack_push(l);
    }
    else
        return this->generate_error(error_invalid_operand(s.kind));
}

void Interpreter::compare(Comparison cmp)
{
    LuaValue b = this->rt->stack_pop();
    LuaValue a = this->rt->stack_pop();
    bool rsl;
    if (a.kind != b.kind || (a.kind != LuaType::LVNumber && a.kind != LuaType::LVString))
    {
        return this->generate_error(error_invalid_comparison(a.kind, b.kind));
    }
    else if (a.kind == LuaType::LVNumber)
        rsl = this->compare_number(a, b, cmp);
    else
        rsl = this->compare_string(a, b, cmp);
    this->push_bool(rsl);
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
    this->compare(Comparison::LT);
}
void Interpreter::i_gt()
{
    this->compare(Comparison::GT);
}
void Interpreter::i_ge()
{
    this->compare(Comparison::GE);
}
void Interpreter::i_le()
{
    this->compare(Comparison::LE);
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
    LuaValue k = this->rt->stack_pop();
    LuaValue t = this->rt->stack_pop();
    LuaValue v = this->rt->table_get(t, k);
    if (this->rt->error_raised())
        this->state = InterpreterState::Error;
    this->rt->stack_push(v);
}
void Interpreter::i_tset()
{
    LuaValue v = this->rt->stack_pop();
    LuaValue k = this->rt->stack_pop();
    LuaValue t = this->rt->stack_back_read(1);
    this->rt->table_set(t, k, v);
    if (this->rt->error_raised())
        this->state = InterpreterState::Error;
}
void Interpreter::i_tnew()
{
    this->rt->stack_push(this->rt->create_table());
}
void Interpreter::i_tlist()
{
    size_t offset = this->arg1;
    size_t count = this->rt->extras();
    LuaValue t = this->rt->stack_back_read(count + 1);
    for (ssize_t i = count - 1; i >= 0; i--)
    {
        LuaValue k = this->rt->create_number(i + offset);
        LuaValue v = this->rt->stack_pop();
        this->rt->table_set(t, k, v);
    }
    this->rt->extras(0);
}
void Interpreter::i_gget()
{
    LuaValue g = this->rt->table_global();
    LuaValue k = this->rt->stack_pop();
    LuaValue v = this->rt->table_get(g, k);
    this->rt->stack_push(v);
}
void Interpreter::i_gset()
{
    LuaValue g = this->rt->table_global();
    LuaValue v = this->rt->stack_pop();
    LuaValue k = this->rt->stack_pop();
    this->rt->table_set(g, k, v);
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
    size_t ip = this->ip;
    this->rt->fncall(this->arg1, this->arg2);
    if (this->rt->error_raised())
        this->state = InterpreterState::Error;
    this->ip = ip;
}
void Interpreter::i_vargs()
{
    size_t count = this->arg1 ? (this->arg1 - 1) : this->rt->argcount();
    for (size_t i = 0; i < count; i++)
        this->rt->stack_push(this->rt->arg(i));
    if (this->arg1 == 0)
        this->rt->extras(count);
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
void Interpreter::generate_error(Lerror error)
{
    LuaValue errval = this->error_to_string(error);
    this->state = InterpreterState::Error;
    this->rt->set_error(errval);
}
LuaValue Interpreter::error_to_string(Lerror error)
{
    if (error.kind == Lerror::LE_InvalidOperand)
    {
        LuaValue s1 = this->rt->create_string("invalid operation on type [");
        LuaValue s2 = this->lua_type_to_string(error.as.invalid_operand.t);
        LuaValue s3 = this->rt->create_string("]");
        return this->concat(s1, this->concat(s2, s3));
    }
    else if (error.kind == Lerror::LE_InvalidComparison)
    {
        LuaValue s1 = this->rt->create_string("attemp to compare ");
        LuaValue s2 = this->lua_type_to_string(error.as.invalid_comparison.t1);
        LuaValue s3 = this->rt->create_string(" with ");
        LuaValue s4 = this->lua_type_to_string(error.as.invalid_comparison.t2);
        return this->concat(s1, this->concat(s2, this->concat(s3, s4)));
    }
    else
    {
        return this->rt->create_nil();
    }
}

LuaValue Interpreter::concat(LuaValue s1, LuaValue s2)
{
    return this->rt->create_string(s1.as<const char *>(), s2.as<const char *>());
}
LuaValue Interpreter::lua_type_to_string(LuaType t)
{
    const char *texts[6] = {
        [LVNil] = "nil",
        [LVBool] = "boolean",
        [LVNumber] = "number",
        [LVString] = "string",
        [LVTable] = "table",
        [LVFunction] = "function",
    };
    return this->rt->create_string(texts[t]);
}