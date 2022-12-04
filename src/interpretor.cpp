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
    Interpretor::optable[IGGet] = &Interpretor::i_gget;
    Interpretor::optable[IGSet] = &Interpretor::i_gset;
    Interpretor::optable[INil] = &Interpretor::i_nil;
    Interpretor::optable[ITrue] = &Interpretor::i_true;
    Interpretor::optable[IFalse] = &Interpretor::i_false;
    Interpretor::optable[ITList] = &Interpretor::i_tlist;
    Interpretor::optable[IRet] = &Interpretor::i_ret;
    Interpretor::optable[IFVargs] = &Interpretor::i_fvargs;
    Interpretor::optable[ICall] = &Interpretor::i_call;
    Interpretor::optable[IVargs] = &Interpretor::i_vargs;
    Interpretor::optable[IJmp] = &Interpretor::i_jmp;
    Interpretor::optable[ICjmp] = &Interpretor::i_cjmp;
    Interpretor::optable[INConst] = &Interpretor::i_const;
    Interpretor::optable[ISConst] = &Interpretor::i_const;
    Interpretor::optable[IFConst] = &Interpretor::i_const;
    Interpretor::optable[ILocal] = &Interpretor::i_local;
    Interpretor::optable[ILStore] = &Interpretor::i_lstore;
    Interpretor::optable[IBLocal] = &Interpretor::i_blocal;
    Interpretor::optable[IBLStore] = &Interpretor::i_blstore;
    Interpretor::optable[IUpvalue] = &Interpretor::i_upvalue;
    Interpretor::optable[IUStore] = &Interpretor::i_ustore;
    Interpretor::optable[IPush] = &Interpretor::i_push;
    Interpretor::optable[IPop] = &Interpretor::i_pop;
}

void Interpretor::load(size_t argc, size_t retc)
{
    Frame *prev = this->rt->frame;
    size_t targcount = prev->retc + argc;
    size_t pidx = prev->sp - targcount - 1;
    Lfunction *fn = (Lfunction *)((prev->stack() + pidx)->data.f->fn);
    this->rt->new_frame(fn->stack_size);
    Frame *frame = this->rt->frame;
    if (targcount > fn->parlen)
    {
        frame->vargsc = targcount - fn->parlen;
        size_t vidx = pidx + 1 + argc;
        for (size_t i = 0; i < frame->vargsc; i++)
            frame->push(prev->stack()[pidx + i]);
    }
    for (size_t i = 0; i < fn->uplen; i++)
    {
        frame->push(this->rt->create_nil());
    }
    frame->sp += fn->parlistsize;
    size_t aidx = pidx + 1;
    for (size_t i = 0; i < fn->parlen; i++)
    {
        if (aidx == pidx + 1 + argc)
            frame->stack()[fn->parmap()[i]] = this->rt->create_nil();
        else
            frame->stack()[fn->parmap()[i]] = prev->stack()[aidx++];
    }
}

void Interpretor::call(Lua *rt, size_t argc, size_t retc)
{
    this->rt = rt;
    this->load(argc, retc);
    while (!this->retc)
    {
        this->fetch();
        this->exec();
    }
    rt->fnret(this->retc);
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

Frame *Interpretor::frame()
{
    return this->rt->frame;
}
Lfunction *Interpretor::bin()
{
    return (Lfunction *)this->frame()->bin();
}
size_t Interpretor::sp()
{
    return this->frame()->sp;
}
void Interpretor::setsp(size_t sp)
{
    this->frame()->sp = sp;
}
LuaValue *Interpretor::stack(size_t idx)
{
    return this->frame()->stack() + idx;
}
Hook *Interpretor::uptable(size_t idx)
{
    return this->frame()->uptable() + idx;
}
Hook *Interpretor::hooktable(size_t idx)
{
    return this->frame()->hooktable() + idx;
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
void Interpretor::i_gget()
{
}
void Interpretor::i_gset()
{
}
void Interpretor::i_nil()
{
}
void Interpretor::i_true()
{
}
void Interpretor::i_false()
{
}
void Interpretor::i_tlist()
{
}
void Interpretor::i_ret()
{
}
void Interpretor::i_fvargs()
{
}
void Interpretor::i_call()
{
}
void Interpretor::i_vargs()
{
}
void Interpretor::i_jmp()
{
}
void Interpretor::i_cjmp()
{
}
void Interpretor::i_const()
{
}
void Interpretor::i_local()
{
}
void Interpretor::i_lstore()
{
}
void Interpretor::i_blocal()
{
}
void Interpretor::i_blstore()
{
}
void Interpretor::i_upvalue()
{
}
void Interpretor::i_ustore()
{
}
void Interpretor::i_push()
{
}
void Interpretor::i_pop()
{
}
