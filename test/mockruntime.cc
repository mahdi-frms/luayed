#include "mockruntime.h"
#include <map>
#include <set>

std::set<string> strset;

#define MOCK_RUNTIME_FAULT_POP 1
#define MOCK_RUNTIME_FAULT_IDX 2
#define MOCK_RUNTIME_FAULT_BACKIDX 3
#define MOCK_RUNTIME_FAULT_ARGIDX 4
#define MOCK_RUNTIME_FAULT_CONSTIDX 5

LuaValue lvnil()
{
    LuaValue v;
    v.kind = LuaType::LVNil;
    return v;
}
LuaValue lvbool(bool b)
{
    LuaValue v;
    v.kind = LuaType::LVBool;
    v.data.b = b;
    return v;
}
LuaValue lvnumber(lnumber n)
{
    LuaValue v;
    v.kind = LuaType::LVNumber;
    v.data.n = n;
    return v;
}
LuaValue lvstring(const char *s)
{
    string str = s;
    auto it = strset.cend();
    if ((it = strset.find(s)) != strset.cend())
    {
        s = it->c_str();
    }
    LuaValue v;
    v.kind = LuaType::LVString;
    v.data.ptr = new string(s);
    return v;
}
LuaValue lvtable()
{
    LuaValue v;
    v.kind = LuaType::LVTable;
    v.data.ptr = new std::map<LuaValue, LuaValue>;
    return v;
}

void Intercept::enable()
{
    this->used = true;
}
void Intercept::enable(size_t arg1)
{
    this->used = true;
    this->arg1 = arg1;
}
void Intercept::enable(size_t arg1, size_t arg2)
{
    this->used = true;
    this->arg1 = arg1;
    this->arg2 = arg2;
}
bool Intercept::check(size_t arg1, size_t arg2)
{
    return this->used && this->arg1 == arg1 && this->arg2 == arg2;
}

size_t MockRuntime::back_stack(size_t idx)
{
    return this->stack.size() - idx;
}
LuaValue MockRuntime::create_nil()
{
    return lvnil();
}
LuaValue MockRuntime::create_boolean(bool b)
{
    return lvbool(b);
}
LuaValue MockRuntime::create_number(lnumber n)
{
    return lvnumber(n);
}
LuaValue MockRuntime::create_string(const char *s)
{
    return lvstring(s);
}
LuaValue MockRuntime::create_table()
{
    return lvtable();
}
LuaValue MockRuntime::create_luafn(fidx_t fidx)
{
    this->icp_luafn.enable(fidx);
    return lvnil();
}
void MockRuntime::fncall(size_t argc, size_t retc)
{
    this->icp_fncall.enable(argc, retc);
}
LuaValue MockRuntime::stack_pop()
{
    if (!this->stack.size())
    {
        throw MOCK_RUNTIME_FAULT_POP;
    }
    LuaValue v = this->stack.back();
    this->stack.pop_back();
    return v;
}
void MockRuntime::stack_push(LuaValue value)
{
    this->stack.push_back(value);
}
LuaValue MockRuntime::stack_read(size_t idx)
{
    if (idx >= this->stack.size())
    {
        throw MOCK_RUNTIME_FAULT_IDX;
    }
    return this->stack[idx];
}
void MockRuntime::stack_write(size_t idx, LuaValue value)
{
    if (idx >= this->stack.size())
    {
        throw MOCK_RUNTIME_FAULT_IDX;
    }
    this->stack[idx] = value;
}
LuaValue MockRuntime::stack_back_read(size_t idx)
{
    if (!idx)
        throw MOCK_RUNTIME_FAULT_BACKIDX;
    idx = this->stack.size() - idx;
    if (idx >= this->stack.size())
    {
        throw MOCK_RUNTIME_FAULT_BACKIDX;
    }
    return this->stack[idx];
}
void MockRuntime::stack_back_write(size_t idx, LuaValue value)
{
    if (!idx)
        throw MOCK_RUNTIME_FAULT_BACKIDX;
    idx = this->stack.size() - idx;
    if (idx >= this->stack.size())
    {
        throw MOCK_RUNTIME_FAULT_BACKIDX;
    }
    this->stack[idx] = value;
}
void MockRuntime::hookpush()
{
    this->icp_hookpush.enable();
}
void MockRuntime::hookpop()
{
    this->icp_hookpop.enable();
}
LuaValue MockRuntime::arg(size_t idx)
{
    if (idx >= this->args.size())
    {
        throw MOCK_RUNTIME_FAULT_ARGIDX;
    }
    return this->args[idx];
}
size_t MockRuntime::load_ip()
{
    this->icp_save_ip.enable();
    return 0;
}
void MockRuntime::save_ip(size_t sp)
{
    this->icp_save_ip.enable(sp);
}
Hook *MockRuntime::upvalue(size_t idx)
{
    // toto
    return nullptr;
}
Hook *MockRuntime::hook(size_t idx)
{
    // toto
    return nullptr;
}
LuaValue MockRuntime::rodata(size_t idx)
{
    if (idx >= this->args.size())
    {
        throw MOCK_RUNTIME_FAULT_CONSTIDX;
    }
    return this->constants[idx];
}
lbyte *MockRuntime::text()
{
    return nullptr;
}

void MockRuntime::set_stack(vector<LuaValue> stack)
{
    this->stack = stack;
}
void MockRuntime::set_constants(vector<LuaValue> constants)
{
    this->constants = constants;
}
void MockRuntime::set_args(vector<LuaValue> args)
{
    this->args = args;
}
void MockRuntime::set_text(vector<Opcode> text)
{
    this->instructions.clear();
    for (size_t i = 0; i < text.size(); i++)
    {
        Opcode op = text[i];
        for (lbyte j = 0; j < op.count; j++)
        {
            this->instructions.push_back(op.bytes[j]);
        }
    }
}
vector<LuaValue> &MockRuntime::get_stack()
{
    return this->stack;
}