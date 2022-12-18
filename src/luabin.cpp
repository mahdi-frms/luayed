#include "luabin.hpp"

Opcode::Opcode(lbyte op, size_t idx1, size_t idx2)
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
Opcode::Opcode(lbyte op, size_t idx)
{
    if (idx >= 256 || op == Instruction::IJmp || op == Instruction::ICjmp)
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
Opcode::Opcode(lbyte op)
{
    this->bytes[0] = op;
    this->count = 1;
}
bool operator==(const Upvalue &l, const Upvalue &r)
{
    return l.fidx == r.fidx && l.offset == r.offset;
}
string binary_stringify(lbyte *text, size_t codelen)
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
    opnames[IUPush] = "upush";
    opnames[IUPop] = "upop";

    opnames[IRet] = opnames[IRet + 1] = "ret";
    opnames[IJmp] = opnames[IJmp + 1] = "jmp";
    opnames[ICjmp] = opnames[ICjmp + 1] = "cjmp";
    opnames[ICall] = opnames[ICall + 1] = opnames[ICall + 2] = opnames[ICall + 3] = "call";
    opnames[IVargs] = opnames[IVargs + 1] = "vargs";
    opnames[ITList] = opnames[ITList + 1] = "tlist";
    opnames[ICall] = opnames[ICall + 1] = "call";
    opnames[IConst] = opnames[IConst + 1] = "C";
    opnames[IFConst] = opnames[IFConst + 1] = "FC";
    opnames[ILocal] = opnames[ILocal + 1] = "local";
    opnames[ILStore] = opnames[ILStore + 1] = "lstore";
    opnames[IBLocal] = opnames[IBLocal + 1] = "blocal";
    opnames[IBLStore] = opnames[IBLStore + 1] = "blstore";
    opnames[IUpvalue] = opnames[IUpvalue + 1] = "upvalue";
    opnames[IUStore] = opnames[IUStore + 1] = "ustore";
    opnames[IPop] = opnames[IPop + 1] = "pop";

    string str;
    for (size_t i = 0; i < codelen; i++)
    {
        lbyte op = text[i];
        str.append(opnames[op]);
        if (op >> 7)
        {
            if (op == ICall)
            {

                int opr1 = text[++i];
                if (op & 0x2)
                    opr1 += text[++i] * 256;

                int opr2 = text[++i];
                if (op & 0x1)
                    opr2 += text[++i] * 256;

                str.push_back(' ');
                str += std::to_string(opr1);
                str.push_back(' ');
                str += std::to_string(opr2);
            }
            else
            {
                int opr = text[++i];
                if (op & 0x01)
                    opr += text[++i] * 256;

                str.push_back(' ');
                str += std::to_string(opr);
            }
        }
        str.push_back('\n');
    }
    return str;
}

Upvalue::Upvalue(fidx_t fidx, size_t offset) : fidx(fidx), offset(offset) {}