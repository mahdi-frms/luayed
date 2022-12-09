#include "generator.hpp"

LuaGenerator::LuaGenerator(Lua *rt) : rt(rt), gfn(nullptr)
{
}

void LuaGenerator::pushf(fidx_t fidx)
{
    GenFunction *gfn = new GenFunction();
    gfn->fidx = fidx;
    gfn->prev = this->gfn;
    this->gfn = gfn;
}
void LuaGenerator::popf()
{
    rt->create_binary(this->gfn);
    GenFunction *prev = this->gfn->prev;
    delete this->gfn;
    this->gfn = prev;
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
size_t LuaGenerator::upval(fidx_t fidx, size_t offset)
{
    size_t idx = this->gfn->upvalues.size();
    this->gfn->upvalues.push_back(Upvalue{.fidx = fidx, .offset = offset});
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
