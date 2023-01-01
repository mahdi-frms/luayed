#include "runtime.h"
#include <cstring>

#define INITIAL_FRAME_SIZE 1024 // must be expandable
#define LV_AS_FUNC(V) ((LuaFunction *)((V)->data.ptr))

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
    return ((LuaFunction *)this->fn.data.ptr)->is_lua;
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

LuaValue LuaRuntime::create_nil()
{
    LuaValue val;
    val.kind = LuaType::LVNil;
    return val;
}

LuaValue LuaRuntime::create_boolean(bool b)
{
    LuaValue val;
    val.kind = LuaType::LVBool;
    val.data.b = b;
    return val;
}

LuaValue LuaRuntime::create_string(const char *s)
{
    LuaValue val;
    val.kind = LuaType::LVString;
    val.data.ptr = nullptr; // todo
    return val;
}

LuaValue LuaRuntime::create_string(const char *s1, const char *s2)
{
    LuaValue val;
    val.kind = LuaType::LVString;
    val.data.ptr = nullptr; // todo
    return val;
}

LuaValue LuaRuntime::create_table()
{
    LuaValue val;
    val.kind = LuaType::LVTable;
    val.data.ptr = nullptr; // todo
    return val;
}

LuaValue LuaRuntime::create_number(lnumber n)
{
    LuaValue val;
    val.kind = LuaType::LVNumber;
    val.data.n = n;
    return val;
}
LuaValue LuaRuntime::create_luafn(fidx_t fidx)
{
    // todo : uptable must be created
    LuaValue val;
    val.kind = LuaType::LVFunction;
    LuaFunction *fobj = (LuaFunction *)this->allocate(sizeof(LuaFunction));
    fobj->is_lua = true;
    fobj->fn = (void *)this->bin(fidx);
    val.data.ptr = (void *)fobj;
    return val;
}
LuaValue LuaRuntime::create_cppfn(LuaRTCppFunction fn)
{
    LuaValue val;
    val.kind = LuaType::LVFunction;
    LuaFunction *fobj = (LuaFunction *)this->allocate(sizeof(LuaFunction));
    fobj->is_lua = false;
    fobj->fn = (void *)fn;
    val.data.ptr = (void *)fobj;
    return val;
}

Lfunction *LuaRuntime::create_binary(GenFunction *gfn)
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

void *LuaRuntime::allocate(size_t size)
{
    return malloc(size);
}
void LuaRuntime::deallocate(void *ptr)
{
    free(ptr);
}
void LuaRuntime::new_frame(size_t stack_size)
{
    Frame *frame = (Frame *)this->allocate(sizeof(Frame) + stack_size);
    frame->ss = stack_size;
    frame->prev = this->frame;
    frame->hookptr = 0;
    frame->sp = 0;
    frame->ret_count = 0;
    frame->fn = this->create_nil();
    this->frame = frame;
}
LuaRuntime::LuaRuntime(IInterpreter *interpreter) : interpreter(interpreter)
{
    this->functable.push_back(nullptr);
    this->new_frame(INITIAL_FRAME_SIZE);
}
void LuaRuntime::destroy_frame()
{
    Frame *frame = this->frame;
    this->frame = frame->prev;
    this->deallocate(frame);
}
void LuaRuntime::copy_values(
    Frame *fsrc,
    Frame *fdest,
    size_t count)
{
    size_t src_idx = fsrc->sp - count;
    for (size_t idx = 0; idx < count; idx++)
    {
        fdest->stack()[fdest->sp + idx] = fsrc->stack()[src_idx + idx];
    }
    fdest->sp += count;
    fsrc->sp -= count;
}

void LuaRuntime::push_nils(
    Frame *frame,
    size_t count)
{
    size_t idx = 0;
    while (count--)
        frame->stack()[frame->sp + idx++] = this->create_nil();
    frame->sp += idx;
}
void LuaRuntime::set_lua_interface(void *lua_interface)
{
    this->lua_interface = lua_interface;
}

void LuaRuntime::fncall(size_t argc, size_t retc)
{
    Frame *prev = this->frame;
    size_t total_argc = argc + prev->ret_count;
    // check if there are enough values on the stack
    if (total_argc + 1 > prev->sp)
    {
        // todo: throw error
        return;
    }
    LuaValue *fn = prev->stack() + prev->sp - total_argc - 1;
    // check if pushed value is a function
    if (fn->kind != LuaType::LVFunction)
    {
        // todo: throw error
        return;
    }
    bool is_lua = fn->as<LuaFunction *>()->is_lua;
    Lfunction *bin = is_lua ? fn->as<LuaFunction *>()->binary() : nullptr;
    this->new_frame(argc + 1024 /* todo: THIS NUMBER MUST BE PROVIDED BY THE COMPILER */);
    Frame *frame = this->frame;
    // move values between frames
    frame->fn = *fn;
    frame->exp_count = retc;
    frame->vargs_count = 0;
    // todo: extra args most not be copied in case of fix return-count
    this->copy_values(prev, frame, total_argc);
    if (is_lua && bin->parcount > total_argc)
    {
        this->push_nils(frame, bin->parcount - total_argc);
    }
    else
    {
        frame->vargs_count += total_argc - (is_lua ? bin->parcount : 0);
    }
    prev->sp--;
    prev->ret_count = 0;
    // execute function
    size_t return_count = 0;
    if (is_lua)
    {
        // todo : handle error returned by interpreter
        if (this->interpreter)
            return_count = this->interpreter->run(this);
        else
        {
            // todo : must be refactored
            std::cerr << "LUA RUNTIME CRASH: NO INTERPRETER PROVIDED";
            exit(1);
        }
    }
    else
    {
        LuaRTCppFunction cppfn = fn->as<LuaFunction *>()->native();
        return_count = cppfn(this->lua_interface);
    }
    this->fnret(return_count);
}
void LuaRuntime::fnret(size_t count)
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
    if (exp-- == 0)
    {
        this->copy_values(frame, prev, total_count);
        prev->ret_count = total_count;
    }
    else if (total_count < exp)
    {
        this->copy_values(frame, prev, total_count);
        this->push_nils(prev, exp - total_count);
    }
    else
        this->copy_values(frame, prev, exp);
    this->destroy_frame();
}

Lfunction *LuaFunction::binary()
{
    return (Lfunction *)this->fn;
}

LuaRTCppFunction LuaFunction::native()
{
    return (LuaRTCppFunction)this->fn;
}

size_t Frame::parcount()
{
    if (this->fn.kind == LuaType::LVNil)
        return 0;
    LuaFunction *fn = this->fn.as<LuaFunction *>();
    return fn->is_lua ? fn->binary()->parcount : 0;
}
size_t Frame::hookmax()
{
    if (this->fn.kind == LuaType::LVNil)
        return 0;
    LuaFunction *fn = this->fn.as<LuaFunction *>();
    return fn->is_lua ? fn->binary()->hookmax : 0;
}
Lfunction *Frame::bin()
{
    return (Lfunction *)this->fn.as<LuaFunction *>()->binary();
}
LuaValue *Frame::stack()
{
    return (LuaValue *)(this->hooktable() + this->hookmax());
}
LuaValue *Frame::vargs()
{
    return (LuaValue *)(this->stack() + this->parcount());
}
size_t Frame::vargcount()
{
    return this->vargs_count;
}
Lfunction *LuaRuntime::bin()
{
    return this->frame->bin();
}
Hook **LuaRuntime::uptable()
{
    return this->frame->uptable();
}
Hook **LuaRuntime::hooktable()
{
    return this->frame->hooktable();
}
Hook **Frame::uptable()
{
    return (Hook **)(((LuaFunction *)this->fn.data.ptr) + 1);
}
Hook **Frame::hooktable()
{
    return (Hook **)(this + 1);
}
LuaValue LuaRuntime::stack_read(size_t idx)
{
    size_t real_idx = this->frame->stack_address(idx);
    return this->frame->stack()[real_idx];
}
size_t Frame::stack_address(size_t idx)
{
    return idx < this->bin()->parcount ? idx : idx + this->vargs_count;
}
void LuaRuntime::stack_write(size_t idx, LuaValue value)
{
    size_t real_idx = this->frame->stack_address(idx);
    this->frame->stack()[real_idx] = value;
}
size_t LuaRuntime::load_ip()
{
    return this->frame->ip;
}
void LuaRuntime::save_ip(size_t ip)
{
    this->frame->ip = ip;
}
LuaValue LuaRuntime::stack_pop()
{
    this->frame->sp--;
    return this->frame->stack()[this->frame->sp];
}
void LuaRuntime::stack_push(LuaValue value)
{
    this->frame->stack()[this->frame->sp] = value;
    this->frame->sp++;
}
void LuaRuntime::hookpush()
{
    this->hooktable()[this->frame->hookptr++] = nullptr;
}
void LuaRuntime::hookpop()
{
    Hook **ptr = this->hooktable() + --this->frame->hookptr;
    Hook hook = **ptr;
    hook.is_detached = true;
    hook.val = *hook.original;
    *ptr = nullptr;
}
LuaValue LuaRuntime::stack_back_read(size_t idx)
{
    idx = this->frame->sp - idx;
    return this->stack_read(idx);
}
void LuaRuntime::stack_back_write(size_t idx, LuaValue value)
{
    idx = this->frame->sp - idx;
    this->stack_write(idx, value);
}
LuaValue LuaRuntime::arg(size_t idx)
{
    LuaValue value = this->frame->vargs()[idx];
    return value;
}
Lfunction *LuaRuntime::bin(size_t fidx)
{
    return this->functable[fidx];
}
Hook *LuaRuntime::upvalue(size_t idx)
{
    return this->uptable()[idx];
}
fidx_t LuaRuntime::gen_fidx()
{
    this->functable.push_back(nullptr);
    return this->functable.size() - 1;
}
LuaValue LuaRuntime::rodata(size_t idx)
{
    return this->bin()->rodata()[idx];
}
lbyte *LuaRuntime::text()
{
    return this->bin()->text();
}
void LuaRuntime::set_error(LuaValue value)
{
    this->frame->error = value;
}
LuaValue LuaRuntime::get_error()
{
    return this->frame->error;
}
size_t LuaRuntime::stack_size()
{
    return this->frame->sp;
}
