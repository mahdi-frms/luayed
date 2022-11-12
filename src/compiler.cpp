#include "compiler.hpp"

char *token_cstring(Token t)
{
    char *str = (char *)malloc(t.len + 1);
    for (size_t i = 0; i < t.len; i++)
        str[i] = t.str[i];
    str[t.len] = '\0';
    return str;
}

char *token_lstring(Token t)
{
    char *str = (char *)malloc(t.len + 1);
    for (size_t i = 0; i < t.len; i++)
        str[i] = t.str[i];
    str[t.len] = '\0';
    return str;
}

lnumber token_number(Token t)
{
    return 0;
}

void Lfunction::push(lbyte b)
{
    this->text.push_back(b);
}
lbyte Lfunction::opcode(size_t index)
{
    return this->text[index];
}
size_t Lfunction::clen()
{
    return this->text.size();
}
size_t Lfunction::number(lnumber n)
{
    size_t i = this->nconst.size();
    this->nconst.push_back(n);
    return i;
}
size_t Lfunction::cstr(const char *s)
{
    size_t i = this->sconst.size();
    this->sconst.push_back(s);
    return i;
}
Lfunction::~Lfunction()
{
    for (size_t i = 0; i < this->sconst.size(); i++)
        free((void *)this->sconst[i]);
}
void Lfunction::push_int(lbyte b, size_t i)
{
    if (i >= 256)
    {
        b |= 0x1;
        this->push(b);
        this->push(i % 256);
        this->push(i >> 8);
    }
    else
    {
        this->push(b);
        this->push(i);
    }
}

vector<Lfunction> Compiler::compile(Ast ast)
{
    this->newf();
    this->compile_node(ast.root());
    this->endf();
    return std::move(this->funcs);
}

lbyte tkn_binops[] = {
    IAdd,
    IMult,
    IFltDiv,
    IFlrDiv,
    IMod,
    IPow,
    IBAnd,
    IBOr,
    ISHR,
    ISHL,
    IConcat,
    ILt,
    ILe,
    IGt,
    IGe,
    IEq,
    INe};

lbyte Compiler::translate_token(TokenKind kind, bool bin)
{
    if (TOKEN_IS_BINARY(kind))
    {
        if (TOKEN_IS_PREFIX(kind))
        {
            return kind == TokenKind::Minus ? (bin ? Instruction::ISub : Instruction::INegate)
                                            : (bin ? Instruction::IBXor : Instruction::IBNot);
        }
        else
        {
            return tkn_binops[kind - 0x0400];
        }
    }
    else // prefix
    {
        return kind == TokenKind::Not ? Instruction::INot : Instruction::ILength;
    }
}

void Compiler::compile_identifier(Noderef node)
{
    MetaDeclaration *md = (MetaDeclaration *)node->getannot(MetaKind::MDecl);
    if (!md)
    {
        size_t idx = this->const_string(token_cstring(node->get_token()));
        this->emit_int(Instruction::ISConst, idx);
        this->emit(Instruction::IGGet);
    }
    else
    {
        Noderef decl = md->decnode;
        MetaMemory *dmd = (MetaMemory *)decl->getannot(MetaKind::MMemory);
        if (dmd->is_stack)
        {
            this->emit_int(Instruction::ILocal, dmd->offset);
        }
        else
        {
            // upval
        }
    }
}

void Compiler::compile_primary(Noderef node)
{
    Token tkn = node->get_token();
    if (tkn.kind == TokenKind::True)
        this->emit(Instruction::ITrue);
    else if (tkn.kind == TokenKind::False)
        this->emit(Instruction::IFalse);
    else if (tkn.kind == TokenKind::Nil)
        this->emit(Instruction::INil);
    else if (tkn.kind == TokenKind::Number)
    {
        size_t idx = this->const_number(token_number(tkn));
        this->emit_int(Instruction::INConst, idx);
    }
    else if (tkn.kind == TokenKind::Literal)
    {
        size_t idx = this->const_string(token_lstring(tkn));
        this->emit_int(Instruction::ISConst, idx);
    }
    else if (tkn.kind == TokenKind::Identifier)
    {
        this->compile_identifier(node);
    }
}

void Compiler::compile_exp(Noderef node)
{
    if (node->get_kind() == NodeKind::Binary)
    {
        this->compile_exp(node->child(0));
        this->compile_exp(node->child(2));
        this->emit(this->translate_token(node->child(1)->get_token().kind, true));
    }
    else if (node->get_kind() == NodeKind::Unary)
    {
        this->compile_exp(node->child(1));
        this->emit(this->translate_token(node->child(0)->get_token().kind, false));
    }
    else if (node->get_kind() == NodeKind::Property)
    {
        this->compile_exp(node->child(0));
        size_t idx = this->const_string(token_cstring(node->child(1)->get_token()));
        this->emit_int(ISConst, idx);
        this->emit(ITGet);
    }
    else if (node->get_kind() == NodeKind::Index)
    {
        this->compile_exp(node->child(0));
        this->compile_exp(node->child(1));
        this->emit(ITGet);
    }
    else if (node->get_kind() == NodeKind::Primary)
    {
        this->compile_primary(node);
    }
}

void Compiler::compile_assignment_primary(Noderef node)
{
    Noderef lvalue = node->child(0)->child(0);
    Noderef rvalue = node->child(1)->child(0);
    MetaDeclaration *md = (MetaDeclaration *)node->getannot(MetaKind::MDecl);

    if (!md)
    {
        const char *str = token_cstring(lvalue->get_token());
        size_t idx = this->const_string(str);
        this->emit_int(Instruction::ISConst, idx);
        this->compile_exp(rvalue);
        this->emit(Instruction::IGGet);
    }
    else
    {
        this->compile_exp(rvalue);
        Noderef decl = md->decnode;
        MetaMemory *dmd = (MetaMemory *)decl->getannot(MetaKind::MMemory);
        if (dmd->is_stack)
        {
            this->emit_int(Instruction::ILStore, dmd->offset);
        }
        else
        {
            // upval
        }
    }
}

void Compiler::compile_assignment(Noderef node)
{
    Noderef lvalue = node->child(0)->child(0);
    Noderef rvalue = node->child(1)->child(0);
    lbyte op;
    size_t op_idx;
    if (lvalue->get_kind() == NodeKind::Primary)
    {
        this->compile_assignment_primary(node);
        return;
    }
    if (lvalue->get_kind() == NodeKind::Property)
    {
        Noderef lexp = lvalue->child(0);
        Noderef prop = lvalue->child(1);
        this->compile_exp(lexp);
        const char *prop_str = token_cstring(prop->get_token());
        size_t idx = this->const_string(prop_str);
        this->emit_int(Instruction::ISConst, idx);
        op = Instruction::ITSet;
    }
    if (lvalue->get_kind() == NodeKind::Index)
    {
        Noderef lexp = lvalue->child(0);
        Noderef iexp = lvalue->child(1);
        this->compile_exp(lexp);
        this->compile_exp(iexp);
        op = Instruction::ITSet;
    }
    this->compile_exp(rvalue);
    if (op == Instruction::ILStore)
        this->emit_int(op, op_idx);
    else
        this->emit(op);
}

void Compiler::compile_block(Noderef node)
{
    MetaScope *md = (MetaScope *)node->getannot(MetaKind::MScope);
    if (md->size)
        this->emit_int(Instruction::IPush, md->size);
    for (size_t i = 0; i < node->child_count(); i++)
        this->compile_node(node->child(i));
    if (md->size)
        this->emit_int(Instruction::IPop, md->size);
}

void Compiler::compile_decl(Noderef node)
{
    Noderef dec = node->child(0)->child(0);
    if (node->child_count() == 1)
    {
        this->emit(Instruction::INil);
    }
    else
    {
        this->compile_exp(node->child(1)->child(0));
    }
    size_t offset = ((MetaMemory *)dec->getannot(MetaKind::MMemory))->offset;
    this->emit(Instruction::INil);
    this->emit_int(Instruction::ILStore, offset);
}

void Compiler::compile_node(Noderef node)
{
    if (node->get_kind() == NodeKind::AssignStmt)
        this->compile_assignment(node);
    else if (node->get_kind() == NodeKind::Block)
        this->compile_block(node);
    else if (node->get_kind() == NodeKind::Declaration)
        this->compile_decl(node);
    else
    {
        exit(4);
    }
}

Lfunction &Compiler::cur()
{
    return this->current.back();
}

void Compiler::newf()
{
    this->current.push_back(std::move(Lfunction()));
}

void Compiler::endf()
{
    this->funcs.push_back(std::move(this->cur()));
    this->current.pop_back();
}

size_t Compiler::const_number(lnumber n)
{
    return this->cur().number(n);
}

size_t Compiler::const_string(const char *s)
{
    return this->cur().cstr(s);
}

void Compiler::emit_int(lbyte b, size_t i)
{
    this->cur().push_int(b, i);
}

void Compiler::emit(lbyte b)
{
    this->cur().push(b);
}