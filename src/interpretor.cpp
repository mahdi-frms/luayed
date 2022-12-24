#include "interpretor.hpp"

opimpl Interpretor::optable[256];

void Interpretor::optable_init()
{
    Interpretor::optable[IAdd] = &Interpretor::i_add;
    Interpretor::optable[ISub] = &Interpretor::i_sub;
    Interpretor::optable[IMult] = &Interpretor::i_mult;
    Interpretor::optable[IFlrDiv] = &Interpretor::i_flrdiv;
    Interpretor::optable[IFltDiv] = &Interpretor::i_fltdiv;
    Interpretor::optable[IMod] = &Interpretor::i_mod;
    Interpretor::optable[IPow] = &Interpretor::i_pow;
    Interpretor::optable[IConcat] = &Interpretor::i_concat;
    Interpretor::optable[IBOr] = &Interpretor::i_bor;
    Interpretor::optable[IBAnd] = &Interpretor::i_band;
    Interpretor::optable[IBXor] = &Interpretor::i_bxor;
    Interpretor::optable[ISHR] = &Interpretor::i_shr;
    Interpretor::optable[ISHL] = &Interpretor::i_shl;
    Interpretor::optable[ILength] = &Interpretor::i_len;
    Interpretor::optable[INegate] = &Interpretor::i_neg;
    Interpretor::optable[INot] = &Interpretor::i_not;
    Interpretor::optable[IBNot] = &Interpretor::i_bnot;
    Interpretor::optable[IEq] = &Interpretor::i_eq;
    Interpretor::optable[INe] = &Interpretor::i_ne;
    Interpretor::optable[IGe] = &Interpretor::i_ge;
    Interpretor::optable[IGt] = &Interpretor::i_gt;
    Interpretor::optable[ILe] = &Interpretor::i_le;
    Interpretor::optable[ILt] = &Interpretor::i_lt;
    Interpretor::optable[ITGet] = &Interpretor::i_tget;
    Interpretor::optable[ITSet] = &Interpretor::i_tset;
    Interpretor::optable[ITNew] = &Interpretor::i_tnew;
    Interpretor::optable[ITList] = &Interpretor::i_tlist;
    Interpretor::optable[IGGet] = &Interpretor::i_gget;
    Interpretor::optable[IGSet] = &Interpretor::i_gset;
    Interpretor::optable[INil] = &Interpretor::i_nil;
    Interpretor::optable[ITrue] = &Interpretor::i_true;
    Interpretor::optable[IFalse] = &Interpretor::i_false;
    Interpretor::optable[IRet] = &Interpretor::i_ret;
    Interpretor::optable[ICall] = &Interpretor::i_call;
    Interpretor::optable[IVargs] = &Interpretor::i_vargs;
    Interpretor::optable[IJmp] = &Interpretor::i_jmp;
    Interpretor::optable[ICjmp] = &Interpretor::i_cjmp;
    Interpretor::optable[IConst] = &Interpretor::i_const;
    Interpretor::optable[IConst] = &Interpretor::i_const;
    Interpretor::optable[IFConst] = &Interpretor::i_fconst;
    Interpretor::optable[ILocal] = &Interpretor::i_local;
    Interpretor::optable[ILStore] = &Interpretor::i_lstore;
    Interpretor::optable[IBLocal] = &Interpretor::i_blocal;
    Interpretor::optable[IBLStore] = &Interpretor::i_blstore;
    Interpretor::optable[IUpvalue] = &Interpretor::i_upvalue;
    Interpretor::optable[IUStore] = &Interpretor::i_ustore;
    Interpretor::optable[IUPush] = &Interpretor::i_upush;
    Interpretor::optable[IUPop] = &Interpretor::i_upop;
    Interpretor::optable[IPop] = &Interpretor::i_pop;
}

size_t Interpretor::run(Lua *rt)
{
    this->rt = rt;
    while (!this->end)
    {
        this->fetch();
        this->exec();
    }
    this->end = false;
    size_t retc = this->retc;
    this->retc = 0;
    return retc;
}

lbyte Interpretor::iread()
{
    return this->bin()->text()[this->ip++];
}

void Interpretor::fetch()
{
    lbyte op = this->iread();
    if (op & 0x80)
    {
        this->arg1 = this->iread();
        if (op & 0x01)
        {
            this->arg1 += this->iread() << 8;
        }

        if (op == Instruction::ICall)
        {
            this->arg2 = this->iread();
            if (op & 0x02)
            {
                this->arg2 += this->iread() << 8;
            }
        }
    }
    this->op = op;
}

void Interpretor::exec()
{
    (this->*optable[this->op])();
}

Lfunction *Interpretor::bin()
{
    return (Lfunction *)this->rt->bin();
}
Hook *Interpretor::upvalue(size_t idx)
{
    return this->rt->uptable()[idx];
}
Hook *Interpretor::hook(size_t idx)
{
    return this->rt->hooktable()[idx];
}

void Interpretor::i_add()
{
}
void Interpretor::i_sub()
{
}
void Interpretor::i_mult()
{
}
void Interpretor::i_flrdiv()
{
}
void Interpretor::i_fltdiv()
{
}
void Interpretor::i_mod()
{
}
void Interpretor::i_pow()
{
}
void Interpretor::i_concat()
{
}
void Interpretor::i_bor()
{
}
void Interpretor::i_band()
{
}
void Interpretor::i_bxor()
{
}
void Interpretor::i_shr()
{
}
void Interpretor::i_shl()
{
}
void Interpretor::i_len()
{
}
void Interpretor::i_neg()
{
}
void Interpretor::i_not()
{
}
void Interpretor::i_bnot()
{
}
void Interpretor::i_lt()
{
}
void Interpretor::i_gt()
{
}
void Interpretor::i_ge()
{
}
void Interpretor::i_le()
{
}
void Interpretor::i_eq()
{
}
void Interpretor::i_ne()
{
}
void Interpretor::i_tget()
{
}
void Interpretor::i_tset()
{
}
void Interpretor::i_tnew()
{
}
void Interpretor::i_tlist()
{
}
void Interpretor::i_gget()
{
}
void Interpretor::i_gset()
{
}
void Interpretor::i_nil()
{
    this->rt->stack_push(this->rt->create_nil());
}
void Interpretor::i_true()
{
    this->rt->stack_push(this->rt->create_boolean(true));
}
void Interpretor::i_false()
{
    this->rt->stack_push(this->rt->create_boolean(false));
}
void Interpretor::i_ret()
{
    this->retc = this->arg1;
    this->end = true;
}
void Interpretor::i_call()
{
    this->rt->save_ip(this->ip);
    this->rt->fncall(this->arg1, this->arg2);
    this->ip = this->rt->load_ip();
}
void Interpretor::i_vargs()
{
}
void Interpretor::i_jmp()
{
    this->ip = this->arg1;
}
void Interpretor::i_cjmp()
{
    LuaValue value = this->rt->stack_pop();
    if (value.truth())
        this->ip = this->arg1;
    this->rt->destroy_value(value);
}
void Interpretor::i_const()
{
}
void Interpretor::i_fconst()
{
}
void Interpretor::i_local()
{
    LuaValue value = this->rt->stack_read(this->arg1);
    this->rt->stack_push(value);
}
void Interpretor::i_lstore()
{
    LuaValue value = this->rt->stack_pop();
    this->rt->stack_write(this->arg1, value);
}
void Interpretor::i_blocal()
{
    LuaValue value = this->rt->stack_back_read(this->arg1);
    this->rt->stack_push(value);
}
void Interpretor::i_blstore()
{
    LuaValue value = this->rt->stack_pop();
    this->rt->stack_back_write(this->arg1, value);
}
void Interpretor::i_upvalue()
{
    Hook *hook = this->upvalue(this->arg1);
    LuaValue value = this->rt->hookread(hook);
    this->rt->stack_push(value);
}
void Interpretor::i_ustore()
{
    Hook *hook = this->upvalue(this->arg1);
    LuaValue value = this->rt->stack_pop();
    this->rt->hookwrite(hook, value);
}
void Interpretor::i_upush()
{
    this->rt->hookpush();
}
void Interpretor::i_upop()
{
    this->rt->hookpop();
}
void Interpretor::i_pop()
{
    for (size_t i = 0; i < this->arg1; i++)
    {
        LuaValue v = this->rt->stack_pop();
        this->rt->destroy_value(v);
    }
}