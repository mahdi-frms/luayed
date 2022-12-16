#include "compiler.hpp"

#define EXPECT_FREE 0xffff

char *token_cstring(Token t)
{
    char *str = new char[t.len + 1];
    for (size_t i = 0; i < t.len; i++)
        str[i] = t.str[i];
    str[t.len] = '\0';
    return str;
}

char *token_lstring(Token t)
{
    char *str = new char[t.len + 1];
    for (size_t i = 0; i < t.len; i++)
        str[i] = t.str[i];
    str[t.len] = '\0';
    return str;
}

lnumber token_number(Token t)
{
    return 0;
}

void Compiler::compile(Noderef root)
{
    MetaScope *fnscp = (MetaScope *)root->getannot(MetaKind::MScope);
    this->gen->pushf(fnscp->fidx);
    this->compile_node(root->get_kind() == NodeKind::Block ? root : root->child(1));
    this->emit(Opcode(Instruction::IRet, 0));
    this->gen->popf();
}

void Compiler::compile(Ast ast)
{
    this->compile(ast.root());
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

size_t Compiler::arglist_count(Noderef arglist)
{
    size_t chlen = arglist->child_count();
    if (!chlen)
        return 0;
    Noderef last = arglist->child(chlen - 1);
    if (last->get_kind() == NodeKind::MethodCall || last->get_kind() == NodeKind::Call)
        chlen--;
    return chlen;
}
size_t Compiler::len()
{
    return this->gen->len();
}

void Compiler::compile_if(Noderef node)
{
    vector<size_t> jmps;
    size_t cjmp = 0;
    for (size_t i = 0; i < node->child_count(); i++)
    {
        Noderef cls = node->child(i);
        if (cls->get_kind() == NodeKind::ElseClause)
        {
            this->compile_block(cls->child(0));
        }
        else
        {
            this->compile_exp(cls->child(0));
            this->emit(Instruction::INot);
            cjmp = this->len();
            this->emit(Opcode(Instruction::ICjmp, 0));
            this->compile_block(cls->child(1));
            jmps.push_back(this->len());
            this->emit(Opcode(Instruction::IJmp, 0));
            size_t cjmp_idx = this->len();
            this->edit_jmp(cjmp, cjmp_idx);
        }
    }
    size_t jmp_idx = this->len();
    for (size_t i = 0; i < jmps.size(); i++)
    {
        this->edit_jmp(jmps[i], jmp_idx);
    }
}

void Compiler::edit_jmp(size_t opidx, size_t jmp_idx)
{
    this->seti(opidx + 1, jmp_idx % 256);
    this->seti(opidx + 2, jmp_idx >> 8);
}

void Compiler::compile_methcall(Noderef node, size_t expect)
{
    this->compile_exp(node->child(0));
    size_t idx = this->const_string(token_cstring(node->child(1)->get_token()));
    this->emit(Opcode(Instruction::IConst, idx));
    this->emit(Opcode(Instruction::ITGet));
    Noderef arglist = node->child(2);
    this->compile_explist(arglist, EXPECT_FREE);
    size_t argcount = this->arglist_count(arglist);
    if (expect == EXPECT_FREE)
        this->emit(Opcode(Instruction::ICall, argcount, 0));
    else
        this->emit(Opcode(Instruction::ICall, argcount, expect + 1));
}
void Compiler::compile_call(Noderef node, size_t expect)
{
    this->compile_exp(node->child(0));
    Noderef arglist = node->child(1);
    size_t argcount = this->arglist_count(arglist);
    this->compile_explist(arglist, EXPECT_FREE);
    if (expect == EXPECT_FREE)
        this->emit(Opcode(Instruction::ICall, argcount, 0));
    else
        this->emit(Opcode(Instruction::ICall, argcount, expect + 1));
}
void Compiler::compile_identifier(Noderef node)
{
    MetaDeclaration *md = (MetaDeclaration *)node->getannot(MetaKind::MDecl);
    Noderef dec = md ? md->decnode : node;
    MetaMemory *mm = (MetaMemory *)dec->getannot(MetaKind::MMemory);
    if (md)
    {
        if (md->is_upvalue)
        {
            MetaScope *fnsc = (MetaScope *)mm->scope->getannot(MetaKind::MScope);
            this->emit(Opcode(Instruction::IUpvalue, this->upval(fnsc->fidx, mm->offset)));
        }
        else
        {
            this->emit(Opcode(Instruction::ILocal, mm->offset));
        }
    }
    else if (node->getannot(MetaKind::MSelf))
    {
        this->emit(Opcode(Instruction::ILocal, 0));
    }
    else
    {
        size_t idx = this->const_string(token_cstring(node->get_token()));
        this->emit(Opcode(Instruction::IConst, idx));
        this->emit(Instruction::IGGet);
    }
}

void Compiler::compile_primary(Noderef node, size_t expect)
{
    Token tkn = node->get_token();
    if (tkn.kind == TokenKind::True)
        this->emit(Instruction::ITrue);
    else if (tkn.kind == TokenKind::DotDotDot)
    {
        if (expect == EXPECT_FREE)
            this->emit(Instruction::IFVargs);
        else
            this->emit(Opcode(Instruction::IVargs, expect));
    }
    else if (tkn.kind == TokenKind::False)
        this->emit(Instruction::IFalse);
    else if (tkn.kind == TokenKind::Nil)
        this->emit(Instruction::INil);
    else if (tkn.kind == TokenKind::Number)
    {
        size_t idx = this->const_number(token_number(tkn));
        this->emit(Opcode(Instruction::IConst, idx));
    }
    else if (tkn.kind == TokenKind::Literal)
    {
        size_t idx = this->const_string(token_lstring(tkn));
        this->emit(Opcode(Instruction::IConst, idx));
    }
    else if (tkn.kind == TokenKind::Identifier)
    {
        this->compile_identifier(node);
    }
}
void Compiler::compile_name(Noderef node)
{
    Token tkn = node->get_token();
    const char *cstr = token_cstring(tkn);
    size_t idx = this->const_string(cstr);
    this->emit(Opcode(Instruction::IConst, idx));
}

void Compiler::compile_table(Noderef node)
{
    this->emit(Opcode(Instruction::ITNew));
    size_t list_len = 0;
    for (size_t i = 0; i < node->child_count(); i++)
    {
        Noderef ch = node->child(i);
        if (ch->get_kind() == NodeKind::IdField)
        {
            this->compile_name(ch->child(0));
            this->compile_exp(ch->child(1));
            this->emit(Instruction::ITSet);
        }
        else if (ch->get_kind() == NodeKind::ExprField)
        {
            this->compile_exp(ch->child(0));
            this->compile_exp(ch->child(1));
            this->emit(Instruction::ITSet);
        }
        else
        {
            this->compile_exp(ch);
            list_len++;
        }
    }
    if (list_len)
    {
        this->emit(Opcode(Instruction::ITList, list_len));
    }
}

void Compiler::compile_exp_e(Noderef node, size_t expect)
{
    bool is_vargs = node->get_kind() == NodeKind::Primary && node->get_token().kind == TokenKind::DotDotDot;
    bool is_fncall = node->get_kind() == NodeKind::Call || node->get_kind() == NodeKind::MethodCall;

    if (!expect && !is_fncall)
        return;
    if (node->get_kind() == NodeKind::Binary)
    {
        TokenKind tk = node->child(1)->get_token().kind;
        if (tk == TokenKind::And || tk == TokenKind::Or)
            this->compile_logic(node);
        else
        {
            this->compile_exp(node->child(0));
            this->compile_exp(node->child(2));
            this->emit(this->translate_token(node->child(1)->get_token().kind, true));
        }
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
        this->emit(Opcode(IConst, idx));
        this->emit(ITGet);
    }
    else if (node->get_kind() == NodeKind::Index)
    {
        this->compile_exp(node->child(0));
        this->compile_exp(node->child(1));
        this->emit(ITGet);
    }
    else if (node->get_kind() == NodeKind::FunctionBody || node->get_kind() == NodeKind::MethodBody)
        this->compile_function(node);
    else if (node->get_kind() == NodeKind::Primary)
        this->compile_primary(node, expect);
    else if (node->get_kind() == NodeKind::Table)
        this->compile_table(node);
    else if (node->get_kind() == NodeKind::Call)
        this->compile_call(node, expect);
    else if (node->get_kind() == NodeKind::MethodCall)
        this->compile_methcall(node, expect);
    if (!is_fncall && !is_vargs && expect != EXPECT_FREE)
        while (--expect)
            this->emit(Opcode(Instruction::INil));
}

Compiler::Compiler(IGenerator *gen) : gen(gen)
{
}

void Compiler::compile_function(Noderef node)
{
    MetaScope *fnscp = (MetaScope *)node->getannot(MetaKind::MScope);
    Compiler compiler(this->gen);
    compiler.stack_offset = node->get_kind() == NodeKind::MethodBody ? 1 : 0;
    compiler.compile(node);
    compiler.emit(Opcode(Instruction::IFConst, fnscp->fidx));
}

void Compiler::compile_exp(Noderef node)
{
    this->compile_exp_e(node, 1);
}

MetaMemory *Compiler::varmem(Noderef lvalue)
{
    MetaDeclaration *md = (MetaDeclaration *)lvalue->getannot(MetaKind::MDecl);
    Noderef dec = md ? md->decnode : lvalue;
    return (MetaMemory *)dec->getannot(MetaKind::MMemory);
}

void Compiler::compile_lvalue_primary(Noderef node)
{
    MetaDeclaration *md = (MetaDeclaration *)node->getannot(MetaKind::MDecl);
    Noderef dec = md ? md->decnode : node;
    MetaMemory *mm = (MetaMemory *)dec->getannot(MetaKind::MMemory);
    if (mm)
    {
        if (md && md->is_upvalue)
        {
            MetaScope *fnsc = (MetaScope *)mm->scope->getannot(MetaKind::MScope);
            this->ops_push(Opcode(Instruction::IUStore, this->upval(fnsc->fidx, mm->offset)));
        }
        else
        {
            this->ops_push(Opcode(Instruction::ILStore, mm->offset));
        }
    }
    else if (node->getannot(MetaKind::MSelf))
    {
        this->ops_push(Opcode(Instruction::ILStore, 0));
    }
    else
    {
        const char *str = token_cstring(node->get_token());
        size_t idx = this->const_string(str);
        this->emit(Opcode(Instruction::IConst, idx));
        this->ops_push(Instruction::IGSet);
        this->vstack.push_back(1);
    }
}

void Compiler::compile_lvalue(Noderef node)
{
    if (node->get_kind() == NodeKind::Primary || node->get_kind() == NodeKind::Name)
    {
        this->compile_lvalue_primary(node);
        return;
    }
    if (node->get_kind() == NodeKind::Property)
    {
        Noderef lexp = node->child(0);
        Noderef prop = node->child(1);
        this->compile_exp(lexp);
        const char *prop_str = token_cstring(prop->get_token());
        size_t idx = this->const_string(prop_str);
        this->emit(Opcode(Instruction::IConst, idx));
        this->ops_push(Opcode(Instruction::ITSet));
        this->vstack.push_back(1);
        this->vstack.push_back(1);
    }
    if (node->get_kind() == NodeKind::Index)
    {
        Noderef lexp = node->child(0);
        Noderef iexp = node->child(1);
        this->compile_exp(lexp);
        this->compile_exp(iexp);
        this->ops_push(Opcode(Instruction::ITSet));
        this->vstack.push_back(1);
        this->vstack.push_back(1);
    }
}

void Compiler::compile_explist(Noderef node, size_t vcount)
{
    if (vcount == EXPECT_FREE)
    {
        if (!node || node->child_count() == 0)
            return;
        for (size_t i = 0; i < node->child_count() - 1; i++)
        {
            this->compile_exp(node->child(i));
        }
        this->compile_exp_e(node->child(node->child_count() - 1), vcount);
    }
    else
    {
        if (!node || node->child_count() == 0)
        {
            while (vcount--)
                this->emit(Opcode(Instruction::INil));
        }
        else
        {
            for (size_t i = 0; i < node->child_count() - 1; i++)
            {
                this->compile_exp_e(node->child(i), vcount ? 1 : 0);
                if (vcount)
                    vcount--;
            }
            this->compile_exp_e(node->child(node->child_count() - 1), vcount);
        }
    }
}

void Compiler::compile_varlist(Noderef node)
{
    if (node->child_count() == 1)
    {
        this->compile_lvalue(node->child(0));
    }
    else
    {
        for (size_t i = 0; i < node->child_count(); i++)
        {
            this->compile_lvalue(node->child(i));
            this->emit(Opcode(Instruction::INil));
            this->vstack.push_back(0);
        }
    }
}

size_t Compiler::vstack_nearest_nil()
{
    size_t i = this->vstack.size() - 1;
    while (this->vstack[i])
        i--;
    this->vstack[i] = 1;
    return this->vstack.size() - 1 - i;
}

void Compiler::compile_generic_for(Noderef node)
{
    Noderef varlist = node->child(0);
    Noderef arglist = node->child(1);
    this->emit(Opcode(Instruction::IPush, varlist->child_count()));
    this->compile_explist(arglist, 3);
    size_t loop_start = this->len();
    this->emit(Opcode(Instruction::IBLocal, 3));
    this->emit(Opcode(Instruction::IBLocal, 3));
    this->emit(Opcode(Instruction::IBLocal, 3));
    this->emit(Opcode(Instruction::ICall, 2, varlist->child_count()));
    for (size_t i = varlist->child_count(); i > 0; i--)
    {
        Noderef var = varlist->child(i - 1)->child(0);
        this->emit(Opcode(Instruction::ILStore, this->varmem(var)->offset));
    }
    this->emit(Opcode(Instruction::ILocal, this->varmem(varlist->child(0)->child(0))->offset));
    this->emit(Opcode(Instruction::IBLStore, 1));
    this->emit(Opcode(Instruction::ILocal, this->varmem(varlist->child(0)->child(0))->offset));
    this->emit(Instruction::INil);
    this->emit(Instruction::IEq);
    size_t cjmp = this->len();
    this->emit(Opcode(Instruction::ICjmp, 0));
    this->loop_start();
    this->compile_block(node->child(2));
    this->emit(Opcode(Instruction::IJmp, loop_start));
    this->edit_jmp(cjmp, this->len());
    this->loop_end();
    this->emit(Opcode(Instruction::IPop, varlist->child_count() + 3));
}
void Compiler::seti(size_t idx, lbyte b)
{
    this->gen->seti(idx, b);
}
size_t Compiler::upval(fidx_t fidx, size_t offset)
{
    return this->gen->upval(fidx, offset);
}

void Compiler::compile_numeric_for(Noderef node)
{
    Noderef lvalue = node->child(0)->child(0);
    Noderef from = node->child(1);
    Noderef to = node->child(2);
    this->emit(Opcode(Instruction::IPush, 1));
    this->compile_exp(from);
    this->emit(Opcode(Instruction::ILStore, this->varmem(lvalue)->offset));
    this->compile_exp(to);
    size_t blk_idx = 3;
    if (node->child_count() == 5)
    {
        blk_idx++;
        this->compile_exp(node->child(3));
    }
    else
        this->emit(Opcode(Instruction::IConst, this->const_number(1)));
    size_t loop_start = this->len();
    this->emit(Opcode(Instruction::IBLocal, 2));
    this->compile_identifier(lvalue);
    this->emit(Instruction::IGt);
    size_t cjmp = this->len();
    this->emit(Opcode(Instruction::ICjmp, 0));
    this->loop_start();
    this->compile_block(node->child(blk_idx));
    this->compile_identifier(lvalue);
    this->emit(Opcode(Instruction::IBLocal, 2));
    this->emit(Instruction::IAdd);
    this->emit(Opcode(Instruction::ILStore, this->varmem(lvalue)->offset));
    this->emit(Opcode(Instruction::IJmp, loop_start));
    this->seti(cjmp, this->len());
    this->loop_end();
    this->emit(Opcode(Instruction::IPop, 3));
}

void Compiler::compile_assignment(Noderef node)
{
    size_t vcount = node->child(0)->child_count();
    this->compile_varlist(node->child(0));
    this->compile_explist(node->child(1), vcount);

    if (vcount > 1)
    {
        size_t v = vcount;
        while (v)
        {
            this->emit(Opcode(Instruction::IBLStore, v + this->vstack_nearest_nil() + 1));
            v--;
        }
    }
    this->vstack.clear();
    this->ops_flush();
}

void Compiler::compile_logic(Noderef node)
{
    this->compile_exp(node->child(0));
    this->emit(Opcode(Instruction::IBLocal, 1));
    if (node->child(1)->get_token().kind == TokenKind::And)
        this->emit(Instruction::INot);
    size_t cjmp = this->len();
    this->emit(Opcode(Instruction::ICjmp, 0));
    this->emit(Opcode(Instruction::IPop, 1));
    this->compile_exp(node->child(2));
    this->seti(cjmp, this->len());
}

void Compiler::compile_block(Noderef node)
{
    MetaScope *md = (MetaScope *)node->getannot(MetaKind::MScope);
    for (size_t i = 0; i < node->child_count(); i++)
        this->compile_node(node->child(i));
    if (md->stack_size)
        this->emit(Opcode(Instruction::IPop, md->stack_size));
}

void Compiler::compile_decl(Noderef node)
{
    Noderef varlist = node->child(0);
    for (size_t i = 0; i < varlist->child_count(); i++)
    {
        Noderef var = varlist->child(i)->child(0);
        MetaMemory *mm = (MetaMemory *)var->getannot(MetaKind::MMemory);
        mm->offset = this->stack_offset++;
    }
    if (node->child_count() == 2)
        this->compile_explist(node->child(1), varlist->child_count());
    else
        for (size_t i = 0; i < varlist->child_count(); i++)
            this->emit(Opcode(Instruction::INil));
}

void Compiler::compile_ret(Noderef node)
{
    if (node->child_count())
    {
        Noderef vals = node->child(0);
        this->compile_explist(vals, EXPECT_FREE);
        this->emit(Opcode(Instruction::IRet, this->arglist_count(vals)));
    }
    else
    {
        this->emit(Opcode(Instruction::IRet, 0));
    }
}

void Compiler::compile_while(Noderef node)
{
    size_t jmp_idx = this->len();
    this->compile_exp(node->child(0));
    this->emit(Instruction::INot);
    size_t cjmp = this->len();
    this->emit(Opcode(Instruction::ICjmp, 0));
    this->loop_start();
    this->compile_block(node->child(1));
    this->emit(Opcode(Instruction::IJmp, jmp_idx));
    this->loop_end();
    this->seti(cjmp, this->len());
}

void Compiler::compile_repeat(Noderef node)
{
    size_t cjmp_idx = this->len();
    this->loop_start();
    this->compile_block(node->child(0));
    this->compile_exp(node->child(1));
    this->emit(Instruction::INot);
    this->emit(Opcode(Instruction::ICjmp, cjmp_idx));
    this->loop_end();
}

void Compiler::compile_break()
{
    this->breaks.push_back(this->len() + 1);
    this->emit(Opcode(Instruction::IJmp, 0));
}

void Compiler::compile_node(Noderef node)
{
    if (node->get_kind() == NodeKind::AssignStmt)
        this->compile_assignment(node);
    else if (node->get_kind() == NodeKind::Block)
        this->compile_block(node);
    else if (node->get_kind() == NodeKind::Declaration)
        this->compile_decl(node);
    else if (node->get_kind() == NodeKind::ReturnStmt)
        this->compile_ret(node);
    else if (node->get_kind() == NodeKind::CallStmt)
        this->compile_exp_e(node->child(0), 0);
    else if (node->get_kind() == NodeKind::IfStmt)
        this->compile_if(node);
    else if (node->get_kind() == NodeKind::WhileStmt)
        this->compile_while(node);
    else if (node->get_kind() == NodeKind::RepeatStmt)
        this->compile_repeat(node);
    else if (node->get_kind() == NodeKind::NumericFor)
        this->compile_numeric_for(node);
    else if (node->get_kind() == NodeKind::GenericFor)
        this->compile_generic_for(node);
    else if (node->get_kind() == NodeKind::BreakStmt)
        this->compile_break();
    else
        exit(4);
}

size_t Compiler::const_number(lnumber n)
{
    return this->gen->const_number(n);
}

size_t Compiler::const_string(const char *s)
{
    return this->gen->const_string(s);
}

void Compiler::emit(Opcode op)
{
    this->gen->emit(op);
}

void Compiler::ops_flush()
{
    while (this->ops.size())
    {
        this->emit(this->ops.back());
        this->ops.pop_back();
    }
}

void Compiler::ops_push(Opcode op)
{
    this->ops.push_back(op);
}

void Compiler::loop_start()
{
    this->breaks.push_back(0);
}
void Compiler::loop_end()
{
    size_t idx = this->len();
    while (this->breaks.back())
    {
        size_t brk = this->breaks.back() - 1;
        this->seti(brk, idx);
        this->breaks.pop_back();
    }
    this->breaks.pop_back();
}