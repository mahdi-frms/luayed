#include "generator.h"
#include "lstrep.h"

using namespace luayed;

void AnalysisGenerator::append(string str)
{
    this->text.append(str);
}
char AnalysisGenerator::hex_digit(size_t num)
{
    if (num > 9)
        return 'a' + (num - 10);
    return '0' + num;
}

void AnalysisGenerator::hex(size_t num)
{
    string s = "0x0000";
    for (size_t i = 0; i < 4; i++)
    {
        s[5 - i] = this->hex_digit(num % 16);
        num /= 16;
    }
    this->append(s);
}

string AnalysisGenerator::stringify()
{
    for (size_t i = 0; i < this->funcs.size(); i++)
    {
        this->fn = this->funcs[i];
        if (this->fn)
        {
            this->fn_stringify();
            this->append("\n");
        }
    }
    return std::move(this->text);
}
void AnalysisGenerator::fn_stringify()
{
    vector<Instruction> inslist;
    this->append("function: ");
    this->append(to_string(this->fn->fidx));
    this->append("\n");
    for (size_t i = 0; i < this->fn->text.size();)
    {
        size_t rc;
        Instruction ins = Instruction::decode(this->fn->text.cbegin().base() + i, &rc);
        inslist.push_back(ins);

        this->hex(i);
        this->append("\t");
        this->append(to_string(ins.op));
        if (ins.oprnd_count() >= 1)
        {
            this->append(" ");
            if (ins.op == Opcode::IJmp || ins.op == Opcode::ICjmp)
                this->hex(ins.oprnd1);
            else
                this->append(to_string(ins.oprnd1));
        }
        if (ins.oprnd_count() >= 2)
        {
            this->append(" ");
            this->append(to_string(ins.oprnd2));
        }

        if (ins.op == Opcode::IConst)
        {
            this->append(" <");
            this->append(this->fn->constants[ins.oprnd1]);
            this->append(">");
        }
        else if (ins.op == Opcode::IUpvalue || ins.op == Opcode::IUStore)
        {
            Upvalue uv = this->fn->upvalues[ins.oprnd1];
            this->append(" <");
            this->append("func idx = ");
            this->append(to_string(uv.fidx));
            this->append("| offset = ");
            this->append(to_string(uv.offset));
            this->append("| hook idx = ");
            this->append(to_string(uv.hidx));
            this->append(">");
        }
        this->append("\n");
        i += rc;
    }
}
