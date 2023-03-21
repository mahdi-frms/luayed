#include "runtime.h"
#include "table.h"
#include <cstdlib>
#include <cstring>
#include "gc.h"

#define LV_AS_FUNC(V) ((LuaFunction *)((V)->data.ptr))

using namespace luayed;

int lstr_compare(const lstr_p &a, const lstr_p &b)
{
    return strcmp(a->cstr(), b->cstr());
}
hash_t lstr_hash(const lstr_p &a)
{
    return a->hash;
}

bool Frame::is_Lua()
{
    return ((LuaFunction *)this->fn.data.ptr)->is_lua;
}

lbyte *Lfunction::text()
{
    return (lbyte *)(this + 1);
}
Upvalue *Lfunction::ups()
{
    return (Upvalue *)(this->innerfns() + this->inlen);
}
LuaValue *Lfunction::rodata()
{
    return (LuaValue *)(this->text() + this->codelen);
}
Lfunction **Lfunction::innerfns()
{
    return (Lfunction **)(this->rodata() + this->rolen);
}
dbginfo_t *Lfunction::dbs()
{
    return (dbginfo_t *)(this->ups() + this->uplen);
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
    return this->create_string(s, "");
}

LuaValue LuaRuntime::create_string(lnumber n)
{
    const size_t buffer_size = 64;
    char buffer[buffer_size];
    snprintf(buffer, buffer_size, "%g", n);
    return this->create_string(buffer);
}

LuaValue LuaRuntime::create_string(const char *s1, const char *s2)
{
    LuaValue val;
    val.kind = LuaType::LVString;

    size_t slen1 = strlen(s1);
    size_t slen2 = strlen(s2);
    size_t strsize = sizeof(lstr_t) + slen1 + slen2 + 1;
    lstr_p str = (lstr_p)this->allocate_raw(strsize);
    strcpy((char *)str->cstr(), s1);
    strcpy((char *)(str->cstr() + slen1), s2);
    *(char *)(str->cstr() + slen1 + slen2) = '\0';
    str->len = slen1 + slen2;
    str->hash = adler32(str->cstr(), str->len);
    lstr_p *p = this->lstrset.get(str);
    if (p)
    {
        this->deallocate_raw(str);
        str = *p;
    }
    else
    {
        lstr_p astr = (lstr_p)this->allocate(strsize, AllocType::ATString);
        memcpy(astr, str, strsize);
        this->deallocate_raw(str);
        str = astr;
        this->lstrset.insert(str);
    }
    val.data.ptr = (void *)str->cstr();
    return val;
}

LuaValue LuaRuntime::create_table()
{
    LuaValue val;
    val.kind = LuaType::LVTable;
    Table *tp = (Table *)this->allocate(sizeof(Table), AllocType::ATTable);
    tp->init(this);
    val.data.ptr = tp;
    return val;
}
bool LuaRuntime::table_check(LuaValue t, LuaValue k, bool is_set)
{
    if (t.kind != LuaType::LVTable)
    {
        this->set_error(this->error_to_string(error_illegal_index(t.kind)));
        return false;
    }
    if (is_set && k.kind == LuaType::LVNil)
    {
        this->set_error(this->error_to_string(error_nil_index()));
        return false;
    }
    return true;
}
void LuaRuntime::table_set(LuaValue t, LuaValue k, LuaValue v)
{
    if (!this->table_check(t, k, true))
        return;
    Table *tp = t.as<Table *>();
    tp->set(k, v);
}
LuaValue LuaRuntime::table_get(LuaValue t, LuaValue k)
{
    if (!this->table_check(t, k, false))
        return this->create_nil();
    if (k.kind == LuaType::LVNil)
        return this->create_nil();
    const Table *tp = t.as<const Table *>();
    LuaValue v = tp->get(k);
    return v;
}
LuaValue LuaRuntime::table_global()
{
    return this->global;
}

size_t LuaRuntime::extras()
{
    return this->frame->ret_count;
}
size_t LuaRuntime::argcount()
{
    return this->frame->vargs_count;
}
LuaValue LuaRuntime::create_number(lnumber n)
{
    LuaValue val;
    val.kind = LuaType::LVNumber;
    val.data.n = n;
    return val;
}
void LuaRuntime::set_compiled_bin(Lfunction *bin)
{
    this->compiled_bin = bin;
}
void LuaRuntime::push_compiled_bin()
{
    LuaFunction *fobj = (LuaFunction *)this->allocate(sizeof(LuaFunction), AllocType::ATFunction);
    fobj->is_lua = true;
    fobj->fn = (void *)this->compiled_bin;
    LuaValue val;
    val.kind = LuaType::LVFunction;
    val.data.ptr = (void *)fobj;
    this->stack_push(val);
}

LuaValue LuaRuntime::create_luafn(fidx_t fidx)
{
    Lfunction *lbin = this->bin()->innerfns()[fidx];
    size_t funcsize = sizeof(LuaFunction) + sizeof(Hook *) * lbin->uplen;
    LuaFunction *fobj = (LuaFunction *)this->allocate(funcsize, AllocType::ATFunction);
    fobj->is_lua = true;
    fobj->fn = (void *)lbin;

    if (this->frame->fn.kind != LuaType::LVNil)
    {
        LuaFunction *parent = this->frame->fn.as<LuaFunction *>();
        if (parent && parent->is_lua)
        {
            Lfunction *pbin = parent->binary();
            for (size_t i = 0; i < lbin->uplen; i++)
            {
                Hook **child_hook = ((Hook **)(fobj + 1)) + i;
                Upvalue child_upvalue = lbin->ups()[i];
                if (child_upvalue.fidx == pbin->fidx)
                {
                    Hook **hook = this->frame->hooktable() + child_upvalue.hidx;
                    if (*hook == nullptr)
                    {
                        *hook = (Hook *)this->allocate(sizeof(Hook), AllocType::ATHook);
                        (*hook)->is_detached = false;
                        (*hook)->original = &this->frame->stack()[this->frame->stack_address(child_upvalue.offset)];
                    }
                    *child_hook = *hook;
                }
                else
                {
                    for (size_t j = 0; j < pbin->uplen; i++)
                    {
                        Hook **parent_hook = ((Hook **)(parent + 1)) + i;
                        Upvalue parent_upvalue = pbin->ups()[i];
                        if (child_upvalue == parent_upvalue)
                        {
                            *child_hook = *parent_hook;
                            break;
                        }
                    }
                }
            }
        }
    }
    LuaValue val;
    val.kind = LuaType::LVFunction;
    val.data.ptr = (void *)fobj;
    return val;
}
LuaValue LuaRuntime::create_cppfn(LuaRTCppFunction fn)
{
    LuaValue val;
    val.kind = LuaType::LVFunction;
    LuaFunction *fobj = (LuaFunction *)this->allocate(sizeof(LuaFunction), AllocType::ATFunction);
    fobj->is_lua = false;
    fobj->fn = (void *)fn;
    val.data.ptr = (void *)fobj;
    return val;
}
LuaValue LuaRuntime::chunkname()
{
    return this->bin()->chunkname;
}

Lfunction *LuaRuntime::create_binary(GenFunction *gfn)
{
    size_t bin_size = sizeof(Lfunction) +
                      gfn->text.size() * sizeof(lbyte) +
                      gfn->rodata.size() * sizeof(LuaValue) +
                      gfn->upvalues.size() * sizeof(Upvalue) +
                      gfn->dbg_lines.size() * sizeof(dbginfo_t) +
                      gfn->innerfns.size() * sizeof(Lfunction *);

    Lfunction *fn = (Lfunction *)this->allocate(bin_size, AllocType::ATBinary);

    fn->fidx = gfn->fidx;
    fn->hookmax = gfn->hookmax;
    fn->parcount = gfn->parcount;
    fn->codelen = gfn->text.size();
    fn->rolen = gfn->rodata.size();
    fn->uplen = gfn->upvalues.size();
    fn->inlen = gfn->innerfns.size();
    fn->dblen = gfn->dbg_lines.size();
    fn->chunkname = gfn->chunkname ? this->create_string(gfn->chunkname) : this->create_nil();

    for (size_t i = 0; i < gfn->text.size(); i++)
        fn->text()[i] = gfn->text[i];
    for (size_t i = 0; i < gfn->rodata.size(); i++)
        fn->rodata()[i] = gfn->rodata[i];
    for (size_t i = 0; i < gfn->upvalues.size(); i++)
        fn->ups()[i] = gfn->upvalues[i];
    for (size_t i = 0; i < gfn->innerfns.size(); i++)
        fn->innerfns()[i] = gfn->innerfns[i];
    for (size_t i = 0; i < gfn->dbg_lines.size(); i++)
        fn->dbs()[i] = gfn->dbg_lines[i];

    return fn;
}
void LuaRuntime::heap_init()
{
    gc_header_t *head = (gc_header_t *)this->allocate_raw(sizeof(gc_header_t));
    gc_header_t *tail = (gc_header_t *)this->allocate_raw(sizeof(gc_header_t));
    head->marked = tail->marked = true;
    head->scan = tail->scan = nullptr;
    head->next = tail->prev = nullptr;
    head->alloc_type = tail->alloc_type = AllocType::ATDummy;
    head->prev = tail;
    tail->next = head;
    this->heap_head = head;
    this->heap_tail = tail;
}
void LuaRuntime::heap_destroy()
{
    this->deallocate_raw(this->heap_head);
    this->deallocate_raw(this->heap_tail);
}
void LuaRuntime::collect_garbage()
{
    GarbageCollector gc;
    gc.run(this);
}
void LuaRuntime::check_garbage_collection()
{
    if (this->allocated > this->threshold)
    {
        this->collect_garbage();
        while (this->allocated > this->threshold)
            this->threshold *= 2;
        while (this->allocated < this->threshold / 4 && this->threshold > 1024)
            this->threshold /= 2;
    }
}
void *LuaRuntime::allocate_raw(size_t size)
{
    this->allocated += size;
    size_t *ptr = (size_t *)malloc(size + sizeof(size_t));
    *ptr = size;
    return ptr + 1;
}
void LuaRuntime::heap_insert(gc_header_t *node, gc_header_t *prev, gc_header_t *next)
{
    prev->next = node;
    next->prev = node;
    node->prev = prev;
    node->next = next;
}
void LuaRuntime::heap_remove(gc_header_t *node)
{
    gc_header_t *prev = node->prev;
    gc_header_t *next = node->next;
    next->prev = prev;
    prev->next = next;
}
void *LuaRuntime::allocate(size_t size, AllocType at)
{
    gc_header_t *obj = (gc_header_t *)this->allocate_raw(size + sizeof(gc_header_t));
    obj->marked = false;
    obj->scan = nullptr;
    obj->alloc_type = at;
    this->heap_insert(obj, this->heap_tail, this->heap_tail->next);
    return obj + 1;
}
void LuaRuntime::deallocate_raw(void *ptr)
{
    size_t *hptr = ((size_t *)ptr) - 1;
    this->allocated -= *hptr;
    free(hptr);
}
void LuaRuntime::new_frame()
{
    Frame *frame;
    if (this->frame)
    {
        frame = (Frame *)(this->frame->stack() + this->stack_size());
    }
    else
    {
        frame = (Frame *)this->stack_buffer;
    }
    frame->prev = this->frame;
    frame->hookptr = 0;
    frame->sp = 0;
    frame->ip = 0;
    frame->ret_count = 0;
    frame->fn = this->create_nil();
    frame->has_error = false;
    frame->error = this->create_nil();
    this->frame = frame;
}
LuaRuntime::LuaRuntime(IInterpreter *interpreter) : interpreter(interpreter)
{
    this->frame = nullptr;
    this->heap_init();
    this->stack_buffer = this->allocate_raw(STACK_BUFFER_SIZE);
    this->lstrset.init(lstr_compare, lstr_hash, this);
    this->func_count = 0;
    this->new_frame();
    this->global = this->create_table();
}
LuaRuntime::~LuaRuntime()
{
    this->frame = nullptr;
    this->global = this->create_nil();
    this->collect_garbage();
    this->heap_destroy();
    this->deallocate_raw(this->stack_buffer);
    this->lstrset.destroy();
}
gc_header_t *LuaRuntime::gc_headers()
{
    return this->heap_tail;
}
void LuaRuntime::deallocate(gc_header_t *hdr)
{
    this->heap_remove(hdr);
    if (hdr->alloc_type == AllocType::ATTable)
    {
        ((Table *)(hdr + 1))->destroy();
    }
    else if (hdr->alloc_type == AllocType::ATString)
    {
        lstr_p str = (lstr_p)(hdr + 1);
        this->lstrset.remove(str);
    }
    this->deallocate_raw(hdr);
}

void LuaRuntime::copy_values(
    Frame *fsrc,
    Frame *fdest,
    size_t count,
    size_t offset = 0)
{
    offset = offset < count ? count : offset;
    size_t src_idx = fsrc->sp - offset;
    size_t dest_idx = fdest->sp;
    LuaValue *sstack = fsrc->stack();
    LuaValue *dstack = fdest->stack();
    for (size_t idx = 0; idx < count; idx++)
    {
        dstack[dest_idx + idx] = sstack[src_idx + idx];
    }
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
LuaValue LuaRuntime::concat(LuaValue v1, LuaValue v2)
{
    return this->create_string(v1.as<const char *>(), v2.as<const char *>());
}
LuaValue LuaRuntime::error_to_string(Lerror error)
{
    if (error.kind == Lerror::LE_NotEnoughArgs)
    {
        LuaValue s1 = this->create_string("expected ");
        LuaValue s2 = this->create_string(error.as.not_enough_args.expected);
        LuaValue s3 = this->create_string(" values, while there are ");
        LuaValue s4 = this->create_string(error.as.not_enough_args.available);
        LuaValue s5 = this->create_string(" on the stack");
        LuaValue s = s1;
        s = this->concat(s, s2);
        s = this->concat(s, s3);
        s = this->concat(s, s4);
        s = this->concat(s, s5);
        return s;
    }
    else if (error.kind == Lerror::LE_CallNonFunction)
    {
        LuaValue s1 = this->create_string("attemp to call a ");
        LuaValue s2 = this->lua_type_to_string(error.as.call_non_function.t);
        LuaValue s3 = this->create_string(" value");
        LuaValue s = s1;
        s = this->concat(s, s2);
        s = this->concat(s, s3);
        return s;
    }
    else if (error.kind == Lerror::LE_NilIndex)
        return this->create_string("table index is nil");
    else if (error.kind == Lerror::LE_IllegalIndex)
    {
        LuaValue s1 = this->create_string("attemp to index a ");
        LuaValue s2 = this->lua_type_to_string(error.as.illegal_index.t);
        LuaValue s3 = this->create_string(" value");
        LuaValue s = s1;
        s = this->concat(s, s2);
        s = this->concat(s, s3);
        return s;
    }
    else
        return this->create_nil();
}

LuaValue LuaRuntime::lua_type_to_string(LuaType t)
{
    const char *texts[6] = {
        [LVNil] = "nil",
        [LVBool] = "boolean",
        [LVNumber] = "number",
        [LVString] = "string",
        [LVTable] = "table",
        [LVFunction] = "function",
    };
    return this->create_string(texts[t]);
}
Fnresult LuaRuntime::fncall_execute()
{
    Fnresult rs;
    LuaFunction *fn = this->frame->fn.as<LuaFunction *>();
    if (fn->is_lua)
    {
        rs = this->interpreter->run(this);
    }
    else
    {
        LuaRTCppFunction cppfn = fn->native();
        size_t return_count = cppfn(this->lua_interface);
        rs.kind = this->frame->has_error ? Fnresult::Error : Fnresult::Ret;
        if (!this->frame->has_error)
            rs.retc = return_count;
        this->check_garbage_collection();
    }
    return rs;
}
Fnresult LuaRuntime::fncall(size_t argc, size_t retc, bool is_tail)
{
    Frame *prev = this->frame;
    if (is_tail)
        retc = prev->exp_count;
    size_t total_argc = argc + prev->ret_count;
    // check if there are enough values on the stack
    if (total_argc + 1 > prev->sp)
    {
        Lerror err = error_not_enough_args(prev->sp - prev->ret_count, argc);
        this->set_error(this->error_to_string(err));
        Fnresult rs;
        rs.kind = Fnresult::Fail;
        return rs;
    }
    LuaValue *fn = prev->stack() + prev->sp - total_argc - 1;
    // check if pushed value is a function
    if (fn->kind != LuaType::LVFunction)
    {
        Lerror err = error_call_non_function(fn->kind);
        this->set_error(this->error_to_string(err));
        Fnresult rs;
        rs.kind = Fnresult::Fail;
        return rs;
    }
    bool is_lua = fn->as<LuaFunction *>()->is_lua;
    Lfunction *bin = is_lua ? fn->as<LuaFunction *>()->binary() : nullptr;
    if (!is_tail)
    {
        // create new frame
        this->new_frame();
        Frame *frame = this->frame;
        frame->fn = *fn;
        frame->exp_count = retc;
        frame->vargs_count = 0;
        // move values between frames
        this->copy_values(prev, frame, total_argc);
        prev->sp -= total_argc;
        frame->sp += total_argc;
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
    }
    else
    {
        while (this->frame->hookptr)
            this->hookpop();
        frame->ip = 0;
        LuaValue *old_stack = frame->stack();
        frame->fn = *fn;
        for (size_t i = 0; i < total_argc; i++)
        {
            frame->stack()[i] = old_stack[frame->sp - total_argc + i];
        }
        frame->sp = total_argc;
        frame->vargs_count = total_argc - (is_lua ? bin->parcount : 0);
        frame->exp_count = retc;
        frame->ret_count = 0;
    }
    // execute function
    return this->fncall_execute();
}
void LuaRuntime::call(size_t argc, size_t retc)
{
    size_t depth = 0;
    Fnresult rs;
    rs.kind = Fnresult::Call;
    rs.argc = argc;
    rs.retc = retc;
    do
    {
        if (rs.kind == Fnresult::Call)
        {
            depth++;
            rs = this->fncall(rs.argc, rs.retc, false);
        }
        else if (rs.kind == Fnresult::Tail)
        {
            rs = this->fncall(rs.argc, this->frame->ret_count, true);
        }
        else
        {
            depth--;
            if (rs.kind != Fnresult::Fail)
                this->fnret(rs.kind == Fnresult::Ret ? rs.retc : 0);
            if (depth)
                rs = this->interpreter->run(this);
        }

    } while (depth);
}
bool LuaRuntime::error_raised()
{
    return this->frame->has_error;
}
bool LuaRuntime::error_metadata()
{
    return this->frame->has_error_meta;
}
void LuaRuntime::error_metadata(bool md)
{
    this->frame->has_error_meta = md;
}
Frame *LuaRuntime::topframe()
{
    return this->frame;
}
void LuaRuntime::store_ip(size_t ip)
{
    this->frame->ip = ip;
}
size_t LuaRuntime::load_ip()
{
    return this->frame->ip;
}
void LuaRuntime::fnret(size_t count)
{
    while (this->frame->hookptr)
        this->hookpop();
    Frame *frame = this->frame;
    size_t total_count = frame->ret_count + count;
    Frame *prev = this->frame->prev;
    prev->error = frame->error;
    prev->has_error = frame->has_error;
    prev->has_error_meta = frame->has_error_meta;
    if (frame->sp < total_count)
    {
        Lerror err = error_not_enough_args(frame->sp - frame->ret_count, count);
        this->set_error(this->error_to_string(err));
        return;
    }
    size_t exp = frame->exp_count;
    if (exp-- == 0)
    {
        this->copy_values(frame, prev, total_count);
        prev->sp += total_count;
        if (this->test_mode || prev->fn.as<LuaFunction *>()->is_lua)
            prev->ret_count = total_count;
    }
    else if (total_count < exp)
    {
        this->copy_values(frame, prev, total_count);
        prev->sp += total_count;
        this->push_nils(prev, exp - total_count);
    }
    else
    {
        this->copy_values(frame, prev, exp, total_count);
        prev->sp += exp;
    }
    this->frame = prev;
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
    return idx < this->bin()->parcount ? idx : (idx + this->vargs_count);
}
void LuaRuntime::stack_write(size_t idx, LuaValue value)
{
    size_t real_idx = this->frame->stack_address(idx);
    this->frame->stack()[real_idx] = value;
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
    Hook *hook = *ptr;
    if (hook)
    {
        hook->is_detached = true;
        hook->val = *hook->original;
        *ptr = nullptr;
    }
}
LuaValue LuaRuntime::stack_back_read(size_t idx)
{
    idx = this->stack_size() - idx;
    return this->frame->stack()[idx];
}
void LuaRuntime::stack_back_write(size_t idx, LuaValue value)
{
    idx = this->stack_size() - idx;
    this->frame->stack()[idx] = value;
}
LuaValue LuaRuntime::arg(size_t idx)
{
    if (idx >= this->frame->vargcount())
        return this->create_nil();
    else
        return this->frame->vargs()[idx];
}
Hook *LuaRuntime::upvalue(size_t idx)
{
    return this->uptable()[idx];
}
fidx_t LuaRuntime::gen_fidx()
{
    return this->func_count++;
}
LuaValue LuaRuntime::rodata(size_t idx)
{
    return this->bin()->rodata()[idx];
}
lbyte *LuaRuntime::text()
{
    return this->bin()->text();
}
dbginfo_t *LuaRuntime::dbgmd()
{
    return this->bin()->dbs();
}
void LuaRuntime::set_error(LuaValue value)
{
    this->frame->error = value;
    this->frame->has_error = true;
    this->frame->has_error_meta = false;
}
LuaValue LuaRuntime::get_error()
{
    return this->frame->error;
}
void LuaRuntime::set_test_mode(bool mode)
{
    this->test_mode = mode;
}
void LuaRuntime::remove_error()
{
    this->frame->has_error = false;
    this->frame->has_error_meta = false;
    this->frame->error = this->create_nil();
}
size_t LuaRuntime::stack_size()
{
    return this->frame->sp;
}
void LuaRuntime::extras(size_t count)
{
    this->frame->ret_count = count;
}
size_t LuaRuntime::length(const char *str)
{
    lstr_p header = ((lstr_p)str) - 1;
    return header->len;
}
