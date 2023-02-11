#include "generator.h"
#include "lstrep.h"

BaseGenerator::BaseGenerator() : current(nullptr), test(nullptr)
{
}
void BaseGenerator::emit(Bytecode opcode)
{
    for (size_t i = 0; i < opcode.count; i++)
        this->current->text.push_back(opcode.bytes[i]);
}
size_t BaseGenerator::len()
{
    return this->current->text.size();
}
void BaseGenerator::debug_info(size_t line)
{
    line++;
    while (this->current->debug.size() <= this->current->text.size())
        this->current->debug.push_back(0);
    this->current->debug.back() = line;
}
void BaseGenerator::seti(size_t idx, lbyte b)
{
    this->current->text[idx] = b;
}
size_t BaseGenerator::const_number(lnumber num)
{
    size_t idx = this->current->constants.size();
    this->current->constants.push_back(to_string(num));
    return idx;
}
size_t BaseGenerator::const_string(const char *str)
{
    size_t idx = this->current->constants.size();
    this->current->constants.push_back(str);
    return idx;
}
fidx_t BaseGenerator::pushf()
{
    FuncTest *fnt = new FuncTest();
    fidx_t fidx = this->fidx_counter++;
    fnt->prev = nullptr;
    fnt->hookmax = 0;
    fnt->parcount = 0;
    fnt->prev = this->current;
    fnt->fidx = fidx;
    this->current = fnt;
    return fidx;
}
void BaseGenerator::popf()
{
    while (this->funcs.size() <= this->current->fidx)
        this->funcs.push_back(nullptr);
    this->funcs[this->current->fidx] = this->current;
    this->current = this->current->prev;
}
size_t BaseGenerator::upval(Upvalue upvalue)
{
    size_t idx = this->current->upvalues.size();
    this->current->upvalues.push_back(upvalue);
    return idx;
}
void BaseGenerator::meta_parcount(size_t parcount)
{
    this->current->parcount = parcount;
}
void BaseGenerator::meta_hookmax(size_t hookmax)
{
    this->current->hookmax = hookmax;
}

void BaseGenerator::meta_chunkname(const char *chunkname)
{
}
BaseGenerator::~BaseGenerator()
{
    for (auto it = this->funcs.begin(); it != this->funcs.end(); it++)
        delete *it;
}

LuaGenerator::LuaGenerator(LuaRuntime *rt) : rt(rt), gfn(nullptr)
{
}

fidx_t LuaGenerator::pushf()
{
    fidx_t fidx = this->gfn ? this->gfn->innerfns.size() : 0;
    GenFunction *gfn = new GenFunction();
    gfn->fidx = fidx;
    gfn->prev = this->gfn;
    this->gfn = gfn;
    return fidx;
}
void LuaGenerator::popf()
{
    Lfunction *bin = rt->create_binary(this->gfn);
    GenFunction *child = this->gfn;
    GenFunction *parent = child->prev;
    if (parent)
        parent->innerfns.push_back(bin);
    else
        this->rt->set_compiled_bin(bin);
    vector<Upvalue> &chups = child->upvalues;
    this->gfn = parent;
    for (size_t i = 0; i < chups.size(); i++)
    {
        Upvalue uv = chups[i];
        if (uv.fidx != parent->fidx)
            this->upval(uv);
    }
    delete child;
}
void LuaGenerator::debug_info(size_t line)
{
    while (this->gfn->dbg_lines.size() <= this->gfn->text.size())
        this->gfn->dbg_lines.push_back(0);
    this->gfn->dbg_lines.back() = line + 1;
}
void LuaGenerator::emit(Bytecode opcode)
{
    for (size_t i = 0; i < opcode.count; i++)
    {
        this->gfn->text.push_back(opcode.bytes[i]);
    }
}
size_t LuaGenerator::len()
{
    return this->gfn->text.size();
}
void LuaGenerator::seti(size_t idx, lbyte b)
{
    this->gfn->text[idx] = b;
}
size_t LuaGenerator::const_number(lnumber num)
{
    return this->add_const(this->rt->create_number(num));
}
size_t LuaGenerator::const_string(const char *str)
{
    return this->add_const(this->rt->create_string(str));
}
size_t LuaGenerator::add_const(LuaValue value)
{
    size_t idx = this->gfn->rodata.size();
    this->gfn->rodata.push_back(value);
    return idx;
}
size_t LuaGenerator::upval(Upvalue upvalue)
{
    size_t idx = this->gfn->upvalues.size();
    this->gfn->upvalues.push_back(upvalue);
    return idx;
}
void LuaGenerator::meta_parcount(size_t parcount)
{
    this->gfn->parcount = parcount;
}
void LuaGenerator::meta_hookmax(size_t hookmax)
{
    this->gfn->hookmax = hookmax;
}
void LuaGenerator::meta_chunkname(const char *chunkname)
{
    this->gfn->chunkname = chunkname;
}