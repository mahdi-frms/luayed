#include "luabin.h"

bool operator==(const Upvalue &l, const Upvalue &r)
{
    return l.fidx == r.fidx && l.offset == r.offset && l.hidx == r.hidx;
}

Upvalue::Upvalue(fidx_t fidx, size_t offset, size_t hidx) : fidx(fidx), offset(offset), hidx(hidx) {}

Instruction::Instruction(Opcode op, size_t oprnd1, size_t oprnd2)
{
    this->op = op;
    this->oprnd1 = oprnd1;
    this->oprnd2 = oprnd2;
}

Bytecode Instruction::encode() const
{
    Bytecode bc;
    lbyte op = this->op;
    bc.count = 1;
    size_t ac = op_oprnd_count(op);
    if (ac >= 1)
    {

        if (this->oprnd1 >= 256 || op == Opcode::IJmp || op == Opcode::ICjmp)
        {
            op = op | 0x1;
            bc.bytes[bc.count++] = this->oprnd1 % 256;
            bc.bytes[bc.count++] = this->oprnd1 >> 8;
        }
        else
        {
            bc.bytes[bc.count++] = this->oprnd1;
        }
    }
    if (ac >= 2)
    {
        if (this->oprnd2 >= 256)
        {
            op = op | 0x2;
            bc.bytes[bc.count++] = this->oprnd2 % 256;
            bc.bytes[bc.count++] = this->oprnd2 >> 8;
        }
        else
        {
            bc.bytes[bc.count++] = this->oprnd2;
        }
    }
    bc.bytes[0] = op;
    return bc;
}

size_t op_oprnd_count(lbyte op)
{
    if (op < 0x80)
        return 0;
    op >>= 1;
    op <<= 1;
    if (op == Opcode::ICall || op == Opcode::ICall + 2)
        return 2;
    return 1;
}

size_t Instruction::oprnd_count() const
{
    return op_oprnd_count(this->op);
}

Instruction Instruction::decode(Bytecode code)
{
    return Instruction::decode(code.bytes);
}

Instruction Instruction::decode(const lbyte *binary, size_t *read_count)
{
    size_t rc = 1;
    lbyte op = binary[0];
    size_t oprnd1 = 0;
    size_t oprnd2 = 0;
    size_t oprnd_count = op_oprnd_count(op);
    if (oprnd_count >= 1)
    {
        oprnd1 = binary[rc++];
        if (op & 0x1)
        {
            oprnd1 += binary[rc++] * 256;
            op -= 0x1;
        }
    }
    if (oprnd_count >= 2)
    {
        oprnd2 = binary[rc++];
        if (op & 0x2)
        {
            oprnd2 += binary[rc++] * 256;
            op -= 0x2;
        }
    }
    if (read_count)
        *read_count = rc;
    return Instruction((Opcode)op, oprnd1, oprnd2);
}