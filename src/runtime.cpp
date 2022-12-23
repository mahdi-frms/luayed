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
bool Frame::is_Lua()
{
    return this->fn.data.f->is_lua;
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

LuaValue Lua::create_boolean(bool b)
{
    LuaValue val;
    val.kind = LuaType::LVBool;
    val.data.b = b;
    return val;
}

LuaValue Lua::create_string(const char *s)
{
    LuaValue val;
    val.kind = LuaType::LVString;
    val.data.s = nullptr; // todo
    return val;
}

LuaValue Lua::create_number(lnumber n)
{
    LuaValue val;
    val.kind = LuaType::LVNumber;
    val.data.n = n;
    return val;
}
LuaValue Lua::create_luafn(Lfunction *bin)
{
    return this->create_nil(); // todo
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
void Lua::new_frame(size_t stack_size)
{
    Frame *frame = (Frame *)this->allocate(sizeof(Frame) + stack_size);
    frame->ss = stack_size;
    frame->prev = this->frame;
    frame->sp = 0;
    frame->ret_count = 0;
    this->frame = frame;
}
void Lua::destroy_frame()
{
    this->destroy_value(this->frame->fn);
    Frame *frame = this->frame;
    this->frame = frame->prev;
    this->deallocate(frame);
}
void Lua::copy_values(
    Frame *fsrc,
    Frame *fdest,
    size_t count)
{
    size_t src_idx = fsrc->sp - count;
    for (size_t idx = 0; idx < count; idx++)
    {
        fdest->stack()[src_idx + idx] = fdest->stack()[fdest->sp + idx];
    }
    fdest->sp += count;
    fsrc->sp -= count;
}

void Lua::push_nils(
    Frame *frame,
    size_t count)
{
    size_t idx = 0;
    while (count--)
        frame->stack()[frame->sp + idx++] = this->create_nil();
    frame->sp += count;
}

void Lua::fncall(size_t argc, size_t retc)
{
    Frame *prev = this->frame;
    size_t total_argc = argc + prev->ret_count;
    // check if there are enough values on the stack
    if (total_argc + 1 > prev->sp)
    {
        // todo: throw error
        return;
    }
    LuaValue *fn = prev->stack() - total_argc - 1;
    bool is_lua = fn->data.f->is_lua;
    Lfunction *bin = is_lua ? (Lfunction *)fn->data.f->fn : nullptr;
    this->new_frame(argc + 1024 /* THIS NUMBER MUST BE PROVIDED BY THE COMPILER */);
    Frame *frame = this->frame;
    // move values bwtween frames
    frame->fn = *fn;
    frame->exp_count = retc;
    frame->vargs_count = 0;
    this->copy_values(prev, frame, total_argc);
    if (is_lua && bin->parcount > total_argc)
        this->push_nils(frame, bin->parcount - total_argc);
    else
        frame->vargs_count += total_argc - bin->parcount;
    prev->sp--;
    prev->ret_count = 0;
    // execute function
    size_t return_count = 0;
    if (is_lua)
    {
        return_count = this->interpretor->call(this);
    }
    else
    {
        LuaCppFunction cppfn = (LuaCppFunction)fn->data.f->fn;
        return_count = cppfn(this);
    }
    this->fnret(return_count);
}
void Lua::fnret(size_t count)
{
    Frame *frame = this->frame;
    if (!frame->prev)
    {
        // todo: error
    }
    size_t total_count = frame->ret_count + count;
    Frame *prev = this->frame->prev;
    if (frame->sp < total_count)
    {
        // todo: error
    }
    size_t exp = frame->exp_count;
    if (exp--)
        this->copy_values(frame, prev, total_count);
    else if (total_count < exp)
    {
        this->copy_values(frame, prev, total_count);
        this->push_nils(frame, exp - total_count);
    }
    else
        this->copy_values(frame, prev, exp);
    if (frame->exp_count)
        prev->ret_count = total_count;
    while (this->stack_ptr())
    {
        LuaValue v = this->stack_pop();
        this->destroy_value(v);
    }
    this->destroy_frame();
}

Lfunction *Frame::bin()
{
    return (Lfunction *)this->fn.data.f->fn;
}
LuaValue *Frame::stack()
{
    return (LuaValue *)(this->hooktable() + this->bin()->hookmax);
}
Lfunction *Lua::bin()
{
    return this->frame->bin();
}
Hook **Lua::uptable()
{
    return this->frame->uptable();
}
Hook **Lua::hooktable()
{
    return this->frame->hooktable();
}
Hook **Frame::uptable()
{
    return (Hook **)(this->fn.data.f + 1);
}
Hook **Frame::hooktable()
{
    return (Hook **)(this + 1);
}
LuaValue Lua::clone_value(LuaValue &value)
{
    return create_nil(); // todo : cloning must be implemented
}
LuaValue Lua::stack_read(size_t idx)
{
    size_t real_idx = this->frame->stack_address(idx);
    return this->clone_value(this->frame->stack()[real_idx]);
}
size_t Frame::stack_address(size_t idx)
{
    return idx < this->bin()->parcount ? idx : idx + this->vargs_count;
}
void Lua::stack_write(size_t idx, LuaValue value)
{
    size_t real_idx = this->frame->stack_address(idx);
    this->destroy_value(this->frame->stack()[real_idx]);
    this->frame->stack()[idx] = value;
}
size_t Lua::stack_ptr()
{
    return this->frame->sp;
}
void Lua::set_stack_ptr(size_t sp)
{
    this->frame->sp = sp;
}
LuaValue Lua::stack_pop()
{
    this->frame->sp--;
    return this->frame->stack()[this->frame->sp];
}
void Lua::stack_push(LuaValue value)
{
    this->frame->stack()[this->frame->sp] = value;
    this->frame->sp++;
}
void Lua::hookpush()
{
    this->hooktable()[this->frame->hookptr++] = nullptr;
}
void Lua::hookpop()
{
    Hook **ptr = this->hooktable() + --this->frame->hookptr;
    Hook hook = **ptr;
    hook.is_detached = true;
    size_t stack_idx = hook.frame->stack_address(hook.stack_idx);
    hook.val = hook.frame->stack()[stack_idx];
    *ptr = nullptr;
}
bool LuaValue::truth()
{
    return this->kind != LuaType::LVNil && (this->kind != LuaType::LVBool || this->data.b);
}
