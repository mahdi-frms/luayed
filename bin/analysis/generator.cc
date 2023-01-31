#include "generator.h"
#include "lstrep.h"

void AnalysisGenerator::append(string str)
{
    this->text.append(str);
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
    this->append(std::to_string(this->fn->fidx));
    for (size_t i = 0; i < this->fn->text.size();)
    {
        size_t rc;
        Instruction ins = Instruction::decode(this->fn->text.cbegin().base() + i, &rc);
        inslist.push_back(ins);

        this->append("\t");
        this->append(to_string(ins.op));
        if (ins.oprnd_count() > 0)
        {
            this->append(" ");
            this->append(to_string(ins.oprnd1));
        }
        if (ins.oprnd_count() > 1)
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
            this->append(std::to_string(uv.fidx));
            this->append("| offset = ");
            this->append(std::to_string(uv.offset));
            this->append("| hook idx = ");
            this->append(std::to_string(uv.hidx));
            this->append(">");
        }
        this->append("\n");
        i += rc;
    }
}
