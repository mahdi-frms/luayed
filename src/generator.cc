#include "generator.h"

LuaGenerator::LuaGenerator(LuaRuntime *rt) : rt(rt), gfn(nullptr)
{
}

fidx_t LuaGenerator::pushf()
{
    fidx_t fidx = this->rt->gen_fidx();
    GenFunction *gfn = new GenFunction();
    if (this->gfn)
        this->gfn->innerfns.push_back(fidx);
    gfn->fidx = fidx;
    gfn->prev = this->gfn;
    this->gfn = gfn;
    return fidx;
}
void LuaGenerator::popf()
{
    rt->create_binary(this->gfn);
    GenFunction *child = this->gfn;
    GenFunction *parent = child->prev;
    vector<Upvalue> &chups = child->upvalues;
    this->gfn = parent;
    for (size_t i = 0; i < chups.size(); i++)
    {
        Upvalue uv = chups[i];
        if (uv.fidx != parent->fidx)
            this->upval(uv.fidx, uv.offset, uv.hidx);
    }
    delete child;
}
void LuaGenerator::debug_info(size_t line)
{
    while (this->gfn->dbg_lines.size() <= this->gfn->text.size())
        this->gfn->dbg_lines.push_back(0);
    this->gfn->dbg_lines.back() = line + 1;
}
void LuaGenerator::emit(Opcode opcode)
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
size_t LuaGenerator::upval(fidx_t fidx, size_t offset, size_t hidx)
{
    size_t idx = this->gfn->upvalues.size();
    this->gfn->upvalues.push_back(Upvalue(fidx, offset, hidx));
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
}