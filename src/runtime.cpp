#include "runtime.hpp"
#include <cstring>

bool operator>(const InternString &l, const InternString &r)
{
    return strcmp(l.lstr, r.lstr) == 1;
}
bool operator<(const InternString &l, const InternString &r)
{
    return strcmp(l.lstr, r.lstr) == -1;
}
bool operator==(const InternString &l, const InternString &r)
{
    return strcmp(l.lstr, r.lstr) == 0;
}
bool operator!=(const InternString &l, const InternString &r)
{
    return strcmp(l.lstr, r.lstr) != 0;
}
bool operator>=(const InternString &l, const InternString &r)
{
    return strcmp(l.lstr, r.lstr) != -1;
}
bool operator<=(const InternString &l, const InternString &r)
{
    return strcmp(l.lstr, r.lstr) != 1;
}

char *StringInterner::insert(char *lstr)
{
    InternString istr = InternString{.lstr = lstr};
    auto it = this->set.find(istr);
    if (it == this->set.cend())
    {
        this->set.insert(istr);
        return lstrnull;
    }
    else
        return it->lstr;
}
void StringInterner::remove(char *lstr)
{
    InternString istr = InternString{.lstr = lstr};
    this->set.erase(this->set.find(istr));
}

lbyte *Lfunction::text()
{
    return (lbyte *)(this + 1);
}
Upvalue *Lfunction::ups()
{
    return (Upvalue *)(this->rodata() + this->rolen);
}
LuaValue *Lfunction::rodata()
{
    return (LuaValue *)(this->text() + this->codelen);
}

string Lfunction::stringify()
{
    const char *opnames[256];
    opnames[IAdd] = "add";
    opnames[ISub] = "sub";
    opnames[IMult] = "mult";
    opnames[IFlrDiv] = "flrdiv";
    opnames[IFltDiv] = "fltdiv";
    opnames[IMod] = "mod";
    opnames[IPow] = "pow";
    opnames[IConcat] = "concat";
    opnames[IBOr] = "or";
    opnames[IBAnd] = "and";
    opnames[IBXor] = "xor";
    opnames[ISHR] = "shl";
    opnames[ISHL] = "shr";
    opnames[ILength] = "length";
    opnames[INegate] = "negate";
    opnames[INot] = "not";
    opnames[IBNot] = "bnot";
    opnames[IEq] = "eq";
    opnames[INe] = "ne";
    opnames[IGe] = "ge";
    opnames[IGt] = "gt";
    opnames[ILe] = "le";
    opnames[ILt] = "lt";
    opnames[ITGet] = "tget";
    opnames[ITSet] = "tset";
    opnames[ITNew] = "tnew";
    opnames[IGGet] = "gget";
    opnames[IGSet] = "gset";
    opnames[INil] = "nil";
    opnames[ITrue] = "true";
    opnames[IFalse] = "false";
    opnames[IFVargs] = "fvargs";

    opnames[IRet] = opnames[IRet + 1] = "ret";
    opnames[IJmp] = opnames[IJmp + 1] = "jmp";
    opnames[ICjmp] = opnames[ICjmp + 1] = "cjmp";
    opnames[ICall] = opnames[ICall + 1] = opnames[ICall + 2] = opnames[ICall + 3] = "call";
    opnames[IVargs] = opnames[IVargs + 1] = "vargs";
    opnames[ITList] = opnames[ITList + 1] = "tlist";
    opnames[ICall] = opnames[ICall + 1] = "call";
    opnames[INConst] = opnames[INConst + 1] = "NC";
    opnames[ISConst] = opnames[ISConst + 1] = "SC";
    opnames[IFConst] = opnames[IFConst + 1] = "FC";
    opnames[ILocal] = opnames[ILocal + 1] = "local";
    opnames[ILStore] = opnames[ILStore + 1] = "lstore";
    opnames[IBLocal] = opnames[IBLocal + 1] = "blocal";
    opnames[IBLStore] = opnames[IBLStore + 1] = "blstore";
    opnames[IUpvalue] = opnames[IUpvalue + 1] = "upvalue";
    opnames[IUStore] = opnames[IUStore + 1] = "ustore";
    opnames[IPush] = opnames[IPush + 1] = "push";
    opnames[IPop] = opnames[IPop + 1] = "pop";

    string text;
    for (size_t i = 0; i < this->codelen; i++)
    {
        lbyte op = this->text()[i];
        text.append(opnames[op]);
        if (op >> 7)
        {
            if (op == ICall)
            {

                int opr1 = this->text()[++i];
                if (op & 0x2)
                    opr1 += this->text()[++i] * 256;

                int opr2 = this->text()[++i];
                if (op & 0x1)
                    opr2 += this->text()[++i] * 256;

                text.push_back(' ');
                text += std::to_string(opr1);
                text.push_back(' ');
                text += std::to_string(opr2);
            }
            else
            {
                int opr = this->text()[++i];
                if (op & 0x01)
                    opr += this->text()[++i] * 256;

                text.push_back(' ');
                text += std::to_string(opr);
            }
        }
        text.push_back('\n');
    }

    return text;
}

LuaValue Lua::create_nil()
{
    LuaValue val;
    val.kind = LuaType::LVNil;
    return val;
}

LuaValue Lua::create_string(const char *s)
{
    LuaValue val;
    val.kind = LuaType::LVString;
    val.data.s = NULL;
    return val;
}

LuaValue Lua::create_number(lnumber n)
{
    LuaValue val;
    val.kind = LuaType::LVString;
    val.data.n = n;
    return val;
}

Lfunction *Lua::create_binary(vector<lbyte> &text,
                              vector<LuaValue> &rodata,
                              vector<Upvalue> &ups,
                              size_t parlen,
                              size_t fidx,
                              size_t stack_size,
                              size_t upvalue_size)
{
    Lfunction *fn = (Lfunction *)this->allocate(
        sizeof(Lfunction) +
        text.size() * sizeof(lbyte) +
        rodata.size() * sizeof(LuaValue) +
        ups.size() * sizeof(Upvalue));

    fn->fidx = fidx;
    fn->parlen = parlen;
    fn->stack_size = stack_size;
    fn->upvalue_size = upvalue_size;
    fn->codelen = text.size();
    fn->rolen = rodata.size();
    fn->uplen = ups.size();

    for (size_t i = 0; i < text.size(); i++)
        fn->text()[i] = text[i];
    for (size_t i = 0; i < rodata.size(); i++)
        fn->rodata()[i] = rodata[i];
    for (size_t i = 0; i < ups.size(); i++)
        fn->ups()[i] = ups[i];

    while (this->functable.size() <= fidx)
        this->functable.push_back(NULL);
    this->functable[fidx] = fn;
    return fn;
}

void *Lua::allocate(size_t size)
{
    return malloc(size);
}
void Lua::deallocate(void *ptr)
{
    free(ptr);
}
LuaValue Frame::pop()
{
    this->sp--;
    return this->stack()[this->sp];
}
void Frame::push(LuaValue value)
{
    this->stack()[this->sp] = value;
    this->sp++;
}
void Lua::new_frame(size_t stack_size)
{
    Frame *frame = (Frame *)this->allocate(sizeof(Frame) + stack_size);
    frame->fn = this->create_nil();
    frame->stack_size = stack_size;
    frame->prev = this->frame;
    frame->sp = 0;
    frame->retc = 0;
    this->frame = frame;
}
void Lua::destroy_frame()
{
    Frame *frame = this->frame;
    this->frame = frame->prev;
    this->deallocate(frame);
}

void Lua::fncall(size_t argc, size_t retc)
{
    this->frame->retc = retc;
    if (argc > this->frame->sp)
    {
        // FIXME: throw error
        return;
    }
    size_t fn_idx = this->frame->sp - argc;
    LuaValue *fn = this->frame->stack() + fn_idx;
    if (fn->data.f->is_lua)
    {
        // FIXME: call intrprt
    }
    else
    {
        this->new_frame(1024 + argc);
        Frame *prev = this->frame->prev;
        for (size_t i = 0; i < argc; i++)
        {
            this->frame->push(prev->stack()[fn_idx + i + 1]);
        }
        for (size_t i = 0; i < prev->vargsc; i++)
        {
            this->frame->push(prev->stack()[prev->sp + i]);
        }
        prev->vargsc = 0;
        prev->sp -= (1 + argc);

        LuaCppFunction cppfn = (LuaCppFunction)fn->data.f->fn;
        size_t retc = cppfn(this);
        this->fnret(retc);
    }
}
void Lua::fnret(size_t count)
{
    if (!this->frame->prev)
    {
        // FIXME: error
    }
    Frame *prev = this->frame->prev;
    if (this->frame->sp < count)
    {
        // FIXME: error
    }
    size_t sidx = this->frame->sp - count;
    if (prev->retc)
    {
        size_t i = 0;
        size_t c = count + prev->vargsc;
        while (--prev->retc)
        {
            if (i < c)
            {
                i++;
                prev->push(this->frame->stack()[sidx + i]);
            }
            else
                prev->push(this->create_nil());
        }
    }
    else
    {
        for (size_t i = 0; i < count + prev->vargsc; i++)
        {
            prev->push(this->frame->stack()[sidx + i]);
        }
    }
    this->destroy_frame();
}
Lfunction *Frame::bin()
{
    return (Lfunction *)this->fn.data.f->fn;
}
LuaValue *Frame::stack()
{
    return (LuaValue *)(this->hooktable() + 32333);
}
Hook *Frame::uptable()
{
    return (Hook *)(this + 1);
}
Hook *Frame::hooktable()
{
    return (Hook *)(this->uptable() + this->bin()->uplen);
}