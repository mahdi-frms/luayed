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
    return binary_stringify(this->text(), this->codelen);
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
    val.data.s = nullptr;
    return val;
}

LuaValue Lua::create_number(lnumber n)
{
    LuaValue val;
    val.kind = LuaType::LVString;
    val.data.n = n;
    return val;
}

Lfunction *Lua::create_binary(GenFunction *gfn)
{
    Lfunction *fn = (Lfunction *)this->allocate(
        sizeof(Lfunction) +
        gfn->text.size() * sizeof(lbyte) +
        gfn->rodata.size() * sizeof(LuaValue) +
        gfn->upvalues.size() * sizeof(Upvalue));

    fn->fidx = gfn->fidx;
    fn->hookmax = gfn->hookmax;
    fn->parcount = gfn->parcount;
    fn->codelen = gfn->text.size();
    fn->rolen = gfn->rodata.size();
    fn->uplen = gfn->upvalues.size();

    for (size_t i = 0; i < gfn->text.size(); i++)
        fn->text()[i] = gfn->text[i];
    for (size_t i = 0; i < gfn->rodata.size(); i++)
        fn->rodata()[i] = gfn->rodata[i];
    for (size_t i = 0; i < gfn->upvalues.size(); i++)
        fn->ups()[i] = gfn->upvalues[i];

    while (this->functable.size() <= gfn->fidx)
        this->functable.push_back(nullptr);
    this->functable[gfn->fidx] = fn;
    return fn;
}

void Lua::destroy_value(LuaValue &value)
{
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
    Frame *prev = this->frame;
    if (argc + 1 > prev->sp - prev->retc)
    {
        // FIXME: throw error
        return;
    }
    size_t pidx = prev->sp - prev->retc - argc - 1;
    LuaValue *fn = prev->stack() + pidx;
    if (fn->data.f->is_lua)
    {
        this->interpretor->call(this, argc, retc);
    }
    else
    {
        prev->retc = retc;
        this->new_frame(1024 + argc);
        Frame *frame = this->frame;
        frame->fn = *fn;
        for (size_t i = 0; i < argc; i++)
        {
            frame->push(prev->stack()[pidx + i + 1]);
        }
        for (size_t i = 0; i < prev->vargsc; i++)
        {
            frame->push(prev->stack()[prev->sp + i]);
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
    Frame *frame = this->frame;
    if (!frame->prev)
    {
        // FIXME: error
    }
    Frame *prev = this->frame->prev;
    if (frame->sp < count)
    {
        // FIXME: error
    }
    size_t sidx = frame->sp - frame->retc - count;
    if (prev->retc)
    {
        size_t i = 0;
        size_t c = count + frame->retc;
        while (--prev->retc)
        {
            if (i < c)
            {
                i++;
                prev->push(frame->stack()[sidx + i]);
            }
            else
                prev->push(this->create_nil());
        }
        while (i != c)
        {
            this->destroy_value(frame->stack()[sidx + i++]);
        }
    }
    else
    {
        for (size_t i = 0; i < count + frame->retc; i++)
        {
            prev->push(frame->stack()[sidx + i]);
        }
        prev->retc = count + frame->retc;
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