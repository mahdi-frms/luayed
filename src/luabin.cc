#include "luabin.h"

Instruction::Instruction(lbyte op, size_t idx1, size_t idx2)
{
    this->count = 1;
    if (idx1 >= 256)
    {
        op = op | 0x2;
        this->bytes[this->count++] = idx1 % 256;
        this->bytes[this->count++] = idx1 >> 8;
    }
    else
    {
        this->bytes[this->count++] = idx1;
    }
    if (idx2 >= 256)
    {
        op = op | 0x1;
        this->bytes[this->count++] = idx2 % 256;
        this->bytes[this->count++] = idx2 >> 8;
    }
    else
    {
        this->bytes[this->count++] = idx2;
    }
    this->bytes[0] = op;
}
Instruction::Instruction(lbyte op, size_t idx)
{
    if (idx >= 256 || op == Opcode::IJmp || op == Opcode::ICjmp)
    {
        this->count = 3;
        this->bytes[0] = op | 0x1;
        this->bytes[1] = idx % 256;
        this->bytes[2] = idx >> 8;
    }
    else
    {
        this->count = 2;
        this->bytes[0] = op;
        this->bytes[1] = idx;
    }
}
Instruction::Instruction(lbyte op)
{
    this->bytes[0] = op;
    this->count = 1;
}
bool operator==(const Upvalue &l, const Upvalue &r)
{
    return l.fidx == r.fidx && l.offset == r.offset && l.hidx == r.hidx;
}

Upvalue::Upvalue(fidx_t fidx, size_t offset, size_t hidx) : fidx(fidx), offset(offset), hidx(hidx) {}